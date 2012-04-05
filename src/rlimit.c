/*-
 * Copyright (c) 2012, Emmanuel Fleury <emmanuel.fleury@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Changelog:
 *  03/26/2012 (Emmanuel Fleury): First public release
 */

#define _BSD_SOURCE		/* needed by wait4 function */
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>

#include "rlimit.h"

#define RETURN_SUCCESS  0
#define RETURN_FAILURE -1

#define CHECK_ERROR(test, msg) \
  if (test) { rlimit_error (msg); goto fail; }
#define CHECK_WARNING(test, msg)\
  if (test) { rlimit_warning (msg); goto fail; }

static char *error = "libsubprocess: error";
static char *warning = "libsubprocess: warning";

static void
rlimit_error (char *msg)
{
  char *s = error;

  /* Concatenate 'error'+': '+'msg' when 'msg != NULL' */
  if (msg)
    {
      if ((s = malloc (snprintf (NULL, 0, "%s: %s", error, msg) + 1)) == NULL)
	s = error;
      else
	sprintf (s, "%s: %s", error, msg);
    }
  perror (s);
}

static void
rlimit_warning (char *msg)
{
  char *s = warning;

  /* Concatenate 'warning'+': '+'msg' when 'msg != NULL' */
  if (msg)
    {
      if ((s = malloc (snprintf (NULL, 0, "%s: %s", warning, msg) + 1)) == NULL)
	s = error;
      else
	sprintf (s, "%s: %s", warning, msg);
    }
  perror (s);
}

subprocess_t *
rlimit_create (int argc, char **argv, char **envp)
{
  subprocess_t *p = malloc (sizeof (subprocess_t));
  /* Handling 'out of memory' */
  CHECK_ERROR ((p == NULL), "subprocess allocation failed");

  /* Initializing the command line arguments */
  p->argc = argc;

  /* Copy argv */
  p->argv = malloc ((argc + 1) * sizeof (char *));
  /* Handling 'out of memory' */
  if (p->argv == NULL)
    {
      free (p);
      CHECK_ERROR (true, "subprocess allocation failed");
    }

  for (int i = 0; i < argc; i++)
    {
      p->argv[i] = malloc ((strlen (argv[i]) + 1) * sizeof (char));
      /* Handling 'out of memory' */
      if (p->argv[i] == NULL)
	{
	  while (i >= 0)
	    free (p->argv[i--]);
	  free (p->argv);
	  free (p);
	  CHECK_ERROR (true, "subprocess allocation failed");
	}

      strcpy (p->argv[i], argv[i]);
    }
  p->argv[argc] = NULL;

  /* Copy envp */
  if (!envp)			/* Handling the case 'envp == NULL' */
    {
      p->envp = NULL;
    }
  else
    {
      int envp_size = 0;
      while (envp[envp_size++]);

      p->envp = malloc ((envp_size + 1) * sizeof (char *));
      /* Handling 'out of memory' */
      if (p->envp == NULL)
	{
	  free (p);
	  CHECK_ERROR (true, "subprocess allocation failed");
	}

      for (int i = 0; i < envp_size; i++)
	{
	  p->envp[i] = malloc (strlen (envp[i]) * sizeof (char));
	  /* Handling 'out of memory' */
	  if (p->envp[i] == NULL)
	    {
	      while (i >= 0)
		free (p->envp[i--]);
	      free (p->envp);
	      free (p);
	      CHECK_ERROR (true, "subprocess allocation failed");
	    }

	  strcpy (p->envp[i], envp[i]);
	}
      p->envp[envp_size] = NULL;
    }

  /* Initializing retval and status */
  p->status = READY;
  p->retval = 0;

  /* Initializing the i/o handlers to NULL */
  p->stdin = NULL;
  p->stdout = NULL;
  p->stderr = NULL;

  /* Initializing the limits and profile to default */
  p->limits = NULL;
  p->profile = NULL;

fail:
  return p;
}

static limits_t *
limits_new (void)
{
  limits_t *limits = malloc (sizeof (limits_t));
  /* Handling 'out of memory' */
  CHECK_ERROR ((limits == NULL), "limits allocation failed");

  /* Default initialization of limits */
  limits->timeout = 0;
  limits->memory = 0;
  limits->fsize = 0;
  limits->fd = 0;
  limits->proc = 0;

  limits->syscalls = malloc (sizeof (int));
  /* Handling 'out of memory' */
  CHECK_ERROR ((limits->syscalls == NULL), "limits allocation failed");

  limits->syscalls[0] = 0;

fail:
  return limits;
}

static void
limits_delete (limits_t * limits)
{
  if (limits)
    {
      free (limits->syscalls);
      free (limits);
    }
}

void
rlimit_delete (subprocess_t * p)
{
  /* Discarding case 'p == NULL' */
  if (!p)
    return;

  /* Handling still non-dead subprocesses */
  /* FIXME: What to do if subprocess is still running ? */
  if (p->status < TERMINATED)
    {
      rlimit_warning ("subprocess was still running");
      rlimit_kill (p);
    }

  /* Freeing argv and envp */
  if (p->argv)
    {
      for (int i = 0; i < p->argc; i++)
	free (p->argv[i]);

      free (p->argv);
    }

  if (p->envp)
    {
      for (int i = 0; p->envp[i] != NULL; i++)
	free (p->envp[i]);

      free (p->envp);
    }

  /* Closing the file descriptors */
  if (p->stdin)
    fclose (p->stdin);

  if (p->stdout)
    fclose (p->stdout);

  if (p->stderr)
    fclose (p->stderr);

  /* Freeing the limits_t */
  if (p->limits)
    limits_delete (p->limits);

  /* Freeing the profile */
  if (p->profile)
    free (p->profile);

  /* Freeing the subprocess */
  free (p);
}

#define CASE(syscall, string) case syscall: fprintf (stdout, string); break;

void
fprintf_syscall (int syscall)
{
  switch (syscall)
    {
      CASE (SYS__sysctl, "sysctl");
      CASE (SYS_access, "access");
      CASE (SYS_acct, "acct");
      CASE (SYS_add_key, "add_key");
      CASE (SYS_adjtimex, "adjtimex");
      CASE (SYS_afs_syscall, "afs_syscall");
      CASE (SYS_alarm, "alarm");
      CASE (SYS_brk, "brk");
      CASE (SYS_capget, "capget");
      CASE (SYS_capset, "capset");
      CASE (SYS_chdir, "chdir");
      CASE (SYS_chmod, "chmod");
      CASE (SYS_chown, "chown");
      CASE (SYS_chroot, "chroot");
      CASE (SYS_clock_adjtime, "clock_adjtime");
      CASE (SYS_clock_getres, "clock_getres");
      CASE (SYS_clock_gettime, "clock_gettime");
      CASE (SYS_clock_nanosleep, "clock_nanosleep");
      CASE (SYS_clock_settime, "clock_settime");
      CASE (SYS_clone, "clone");
      CASE (SYS_close, "close");
      CASE (SYS_creat, "creat");
      CASE (SYS_create_module, "create_module");
      CASE (SYS_delete_module, "delete_module");
      CASE (SYS_dup, "dup");
      CASE (SYS_dup2, "dup2");
      CASE (SYS_dup3, "dup3");
      CASE (SYS_epoll_create, "epoll_create");
      CASE (SYS_epoll_create1, "epoll_create1");
      CASE (SYS_epoll_ctl, "epoll_ctl");
      CASE (SYS_epoll_pwait, "epoll_pwait");
      CASE (SYS_epoll_wait, "epoll_wait");
      CASE (SYS_eventfd, "eventfd");
      CASE (SYS_eventfd2, "eventfd2");
      CASE (SYS_execve, "execve");
      CASE (SYS_exit, "exit");
      CASE (SYS_exit_group, "exit_group");
      CASE (SYS_faccessat, "faccessat");
      CASE (SYS_fadvise64, "fadvise64");
      CASE (SYS_fallocate, "fallocate");
      CASE (SYS_fanotify_init, "fanotify_init");
      CASE (SYS_fanotify_mark, "fanotify_mark");
      CASE (SYS_fchdir, "fchdir");
      CASE (SYS_fchmod, "fchmod");
      CASE (SYS_fchmodat, "fchmodat");
      CASE (SYS_fchown, "fchown");
      CASE (SYS_fchownat, "fchownat");
      CASE (SYS_fcntl, "fcntl");
      CASE (SYS_fdatasync, "fdatasync");
      CASE (SYS_fgetxattr, "fgetxattr");
      CASE (SYS_flistxattr, "flistxattr");
      CASE (SYS_flock, "flock");
      CASE (SYS_fork, "fork");
      CASE (SYS_fremovexattr, "fremovexattr");
      CASE (SYS_fsetxattr, "fsetxattr");
      CASE (SYS_fstat, "fstat");
      CASE (SYS_fsync, "fsync");
      CASE (SYS_ftruncate, "ftruncate");
      CASE (SYS_futex, "futex");
      CASE (SYS_futimesat, "futimesat");
      CASE (SYS_get_kernel_syms, "get_kernel_syms");
      CASE (SYS_get_mempolicy, "get_mempolicy");
      CASE (SYS_get_robust_list, "get_robust_list");
      CASE (SYS_get_thread_area, "get_thread_area");
      CASE (SYS_getcpu, "getcpu");
      CASE (SYS_getcwd, "getcwd");
      CASE (SYS_getdents64, "getdents64");
      CASE (SYS_getegid, "getegid");
      CASE (SYS_geteuid, "geteuid");
      CASE (SYS_getgid, "getgid");
      CASE (SYS_getgroups, "getgroups");
      CASE (SYS_getitimer, "getitimer");
      CASE (SYS_getpgid, "getpgid");
      CASE (SYS_getpgrp, "getpgrp");
      CASE (SYS_getpid, "getpid");
      CASE (SYS_getpmsg, "getpmsg");
      CASE (SYS_getppid, "getppid");
      CASE (SYS_getpriority, "getpriority");
      CASE (SYS_getresgid, "getresgid");
      CASE (SYS_getresuid, "getresuid");
      CASE (SYS_getrlimit, "getrlimit");
      CASE (SYS_getrusage, "getrusage");
      CASE (SYS_getsid, "getsid");
      CASE (SYS_gettid, "gettid");
      CASE (SYS_gettimeofday, "gettimeofday");
      CASE (SYS_getuid, "getuid");
      CASE (SYS_getxattr, "getxattr");
      CASE (SYS_init_module, "init_module");
      CASE (SYS_inotify_add_watch, "inotify_add_watch");
      CASE (SYS_inotify_init, "inotify_init");
      CASE (SYS_inotify_init1, "inotify_init1");
      CASE (SYS_inotify_rm_watch, "inotify_rm_watch");
      CASE (SYS_io_cancel, "io_cancel");
      CASE (SYS_io_destroy, "io_destroy");
      CASE (SYS_io_getevents, "io_getevents");
      CASE (SYS_io_setup, "io_setup");
      CASE (SYS_io_submit, "io_submit");
      CASE (SYS_ioctl, "ioctl");
      CASE (SYS_ioperm, "ioperm");
      CASE (SYS_iopl, "iopl");
      CASE (SYS_ioprio_get, "ioprio_get");
      CASE (SYS_ioprio_set, "ioprio_set");
      CASE (SYS_kexec_load, "kexec_load");
      CASE (SYS_keyctl, "keyctl");
      CASE (SYS_kill, "kill");
      CASE (SYS_lchown, "lchown");
      CASE (SYS_lgetxattr, "lgetxattr");
      CASE (SYS_link, "link");
      CASE (SYS_linkat, "linkat");
      CASE (SYS_listxattr, "listxattr");
      CASE (SYS_llistxattr, "llistxattr");
      CASE (SYS_lookup_dcookie, "lookup_dcookie");
      CASE (SYS_lremovexattr, "lremovexattr");
      CASE (SYS_lseek, "lseek");
      CASE (SYS_lsetxattr, "lsetxattr");
      CASE (SYS_lstat, "lstat");
      CASE (SYS_madvise, "madvise");
      CASE (SYS_mbind, "mbind");
      CASE (SYS_migrate_pages, "migrate_pages");
      CASE (SYS_mincore, "mincore");
      CASE (SYS_mkdir, "mkdir");
      CASE (SYS_mkdirat, "mkdirat");
      CASE (SYS_mknod, "mknod");
      CASE (SYS_mknodat, "mknodat");
      CASE (SYS_mlock, "mlock");
      CASE (SYS_mlockall, "mlockall");
      CASE (SYS_mmap, "mmap");
      CASE (SYS_modify_ldt, "modify_ldt");
      CASE (SYS_mount, "mount");
      CASE (SYS_move_pages, "move_pages");
      CASE (SYS_mprotect, "mprotect");
      CASE (SYS_mq_getsetattr, "mq_getsetattr");
      CASE (SYS_mq_notify, "mq_notify");
      CASE (SYS_mq_open, "mq_open");
      CASE (SYS_mq_timedreceive, "mq_timedreceive");
      CASE (SYS_mq_timedsend, "mq_timedsend");
      CASE (SYS_mq_unlink, "mq_unlink");
      CASE (SYS_mremap, "mremap");
      CASE (SYS_msync, "msync");
      CASE (SYS_munlock, "munlock");
      CASE (SYS_munlockall, "munlockall");
      CASE (SYS_munmap, "munmap");
      CASE (SYS_name_to_handle_at, "name_to_handle_at");
      CASE (SYS_nanosleep, "nanosleep");
      CASE (SYS_nfsservctl, "nfsservctl");
      CASE (SYS_open, "open");
      CASE (SYS_open_by_handle_at, "open_by_handle_at");
      CASE (SYS_openat, "openat");
      CASE (SYS_pause, "pause");
      CASE (SYS_perf_event_open, "perf_event_open");
      CASE (SYS_personality, "personality");
      CASE (SYS_pipe, "pipe");
      CASE (SYS_pipe2, "pipe2");
      CASE (SYS_pivot_root, "pivot_root");
      CASE (SYS_poll, "poll");
      CASE (SYS_ppoll, "ppoll");
      CASE (SYS_prctl, "prctl");
      CASE (SYS_pread64, "pread64");
      CASE (SYS_preadv, "preadv");
      CASE (SYS_prlimit64, "prlimit64");
      CASE (SYS_process_vm_readv, "process_vm_readv");
      CASE (SYS_process_vm_writev, "process_vm_writev");
      CASE (SYS_pselect6, "pselect6");
      CASE (SYS_ptrace, "ptrace");
      CASE (SYS_putpmsg, "putpmsg");
      CASE (SYS_pwrite64, "pwrite64");
      CASE (SYS_pwritev, "pwritev");
      CASE (SYS_query_module, "query_module");
      CASE (SYS_quotactl, "quotactl");
      CASE (SYS_read, "read");
      CASE (SYS_readahead, "readahead");
      CASE (SYS_readlink, "readlink");
      CASE (SYS_readlinkat, "readlinkat");
      CASE (SYS_readv, "readv");
      CASE (SYS_reboot, "reboot");
      CASE (SYS_recvmmsg, "recvmmsg");
      CASE (SYS_remap_file_pages, "remap_file_pages");
      CASE (SYS_removexattr, "removexattr");
      CASE (SYS_rename, "rename");
      CASE (SYS_renameat, "renameat");
      CASE (SYS_request_key, "request_key");
      CASE (SYS_restart_syscall, "restart_syscall");
      CASE (SYS_rmdir, "rmdir");
      CASE (SYS_rt_sigaction, "rt_sigaction");
      CASE (SYS_rt_sigpending, "rt_sigpending");
      CASE (SYS_rt_sigprocmask, "rt_sigprocmask");
      CASE (SYS_rt_sigqueueinfo, "rt_sigqueueinfo");
      CASE (SYS_rt_sigreturn, "rt_sigreturn");
      CASE (SYS_rt_sigsuspend, "rt_sigsuspend");
      CASE (SYS_rt_sigtimedwait, "rt_sigtimedwait");
      CASE (SYS_rt_tgsigqueueinfo, "rt_tgsigqueueinfo");
      CASE (SYS_sched_get_priority_max, "sched_get_priority_max");
      CASE (SYS_sched_get_priority_min, "sched_get_priority_min");
      CASE (SYS_sched_getaffinity, "sched_getaffinity");
      CASE (SYS_sched_getparam, "sched_getparam");
      CASE (SYS_sched_getscheduler, "sched_getscheduler");
      CASE (SYS_sched_rr_get_interval, "sched_rr_get_interval");
      CASE (SYS_sched_setaffinity, "sched_setaffinity");
      CASE (SYS_sched_setparam, "sched_setparam");
      CASE (SYS_sched_setscheduler, "sched_setscheduler");
      CASE (SYS_sched_yield, "sched_yield");
      CASE (SYS_select, "select");
      CASE (SYS_sendfile, "sendfile");
      CASE (SYS_sendmmsg, "sendmmsg");
      CASE (SYS_set_mempolicy, "set_mempolicy");
      CASE (SYS_set_robust_list, "set_robust_list");
      CASE (SYS_set_thread_area, "set_thread_area");
      CASE (SYS_set_tid_address, "set_tid_address");
      CASE (SYS_setdomainname, "setdomainname");
      CASE (SYS_setfsgid, "setfsgid");
      CASE (SYS_setfsuid, "setfsuid");
      CASE (SYS_setgid, "setgid");
      CASE (SYS_setgroups, "setgroups");
      CASE (SYS_sethostname, "sethostname");
      CASE (SYS_setitimer, "setitimer");
      CASE (SYS_setns, "setns");
      CASE (SYS_setpgid, "setpgid");
      CASE (SYS_setpriority, "setpriority");
      CASE (SYS_setregid, "setregid");
      CASE (SYS_setresgid, "setresgid");
      CASE (SYS_setresuid, "setresuid");
      CASE (SYS_setreuid, "setreuid");
      CASE (SYS_setrlimit, "setrlimit");
      CASE (SYS_setsid, "setsid");
      CASE (SYS_settimeofday, "settimeofday");
      CASE (SYS_setuid, "setuid");
      CASE (SYS_setxattr, "setxattr");
      CASE (SYS_sigaltstack, "sigaltstack");
      CASE (SYS_signalfd, "signalfd");
      CASE (SYS_signalfd4, "signalfd4");
      CASE (SYS_splice, "splice");
      CASE (SYS_stat, "stat");
      CASE (SYS_statfs, "statfs");
      CASE (SYS_swapoff, "swapoff");
      CASE (SYS_swapon, "swapon");
      CASE (SYS_symlink, "symlink");
      CASE (SYS_symlinkat, "symlinkat");
      CASE (SYS_sync, "sync");
      CASE (SYS_sync_file_range, "sync_file_range");
      CASE (SYS_syncfs, "syncfs");
      CASE (SYS_sysfs, "sysfs");
      CASE (SYS_sysinfo, "sysinfo");
      CASE (SYS_syslog, "syslog");
      CASE (SYS_tee, "tee");
      CASE (SYS_tgkill, "tgkill");
      CASE (SYS_time, "time");
      CASE (SYS_timer_create, "timer_create");
      CASE (SYS_timer_delete, "timer_delete");
      CASE (SYS_timer_getoverrun, "timer_getoverrun");
      CASE (SYS_timer_gettime, "timer_gettime");
      CASE (SYS_timer_settime, "timer_settime");
      CASE (SYS_timerfd_create, "timerfd_create");
      CASE (SYS_timerfd_gettime, "timerfd_gettime");
      CASE (SYS_timerfd_settime, "timerfd_settime");
      CASE (SYS_times, "times");
      CASE (SYS_tkill, "tkill");
      CASE (SYS_truncate, "truncate");
      CASE (SYS_umask, "umask");
      CASE (SYS_umount2, "umount2");
      CASE (SYS_uname, "uname");
      CASE (SYS_unlink, "unlink");
      CASE (SYS_unlinkat, "unlinkat");
      CASE (SYS_unshare, "unshare");
      CASE (SYS_uselib, "uselib");
      CASE (SYS_ustat, "ustat");
      CASE (SYS_utime, "utime");
      CASE (SYS_utimensat, "utimensat");
      CASE (SYS_utimes, "utimes");
      CASE (SYS_vfork, "vfork");
      CASE (SYS_vhangup, "vhangup");
      CASE (SYS_vmsplice, "vmsplice");
      CASE (SYS_vserver, "vserver");
      CASE (SYS_wait4, "wait4");
      CASE (SYS_waitid, "waitid");
      CASE (SYS_write, "write");
      CASE (SYS_writev, "writev");
#if __WORDSIZE == 64
      CASE (SYS_accept, "accept");
      CASE (SYS_accept4, "accept4");
      CASE (SYS_arch_prctl, "arch_prctl");
      CASE (SYS_bind, "bind");
      CASE (SYS_connect, "connect");
      CASE (SYS_epoll_ctl_old, "epoll_ctl_old");
      CASE (SYS_epoll_wait_old, "epoll_wait_old");
      CASE (SYS_getpeername, "getpeername");
      CASE (SYS_getsockname, "getsockname");
      CASE (SYS_getsockopt, "getsockopt");
      CASE (SYS_listen, "listen");
      CASE (SYS_msgctl, "msgctl");
      CASE (SYS_msgget, "msgget");
      CASE (SYS_msgrcv, "msgrcv");
      CASE (SYS_msgsnd, "msgsnd");
      CASE (SYS_newfstatat, "newfstatat");
      CASE (SYS_recvfrom, "recvfrom");
      CASE (SYS_recvmsg, "recvmsg");
      CASE (SYS_security, "security");
      CASE (SYS_semctl, "semctl");
      CASE (SYS_semget, "semget");
      CASE (SYS_semop, "semop");
      CASE (SYS_semtimedop, "semtimedop");
      CASE (SYS_sendmsg, "sendmsg");
      CASE (SYS_sendto, "sendto");
      CASE (SYS_setsockopt, "setsockopt");
      CASE (SYS_shmat, "shmat");
      CASE (SYS_shmctl, "shmctl");
      CASE (SYS_shmdt, "shmdt");
      CASE (SYS_shmget, "shmget");
      CASE (SYS_shutdown, "shutdown");
      CASE (SYS_socket, "socket");
      CASE (SYS_socketpair, "socketpair");
      CASE (SYS_tuxcall, "tuxcall");
#else /* WORDSIZE == 32 */
      CASE (SYS__llseek, "_llseek");
      CASE (SYS__newselect, "_newselect");
      CASE (SYS_bdflush, "bdflush");
      CASE (SYS_break, "break");
      CASE (SYS_chown32, "chown32");
      CASE (SYS_fadvise64_64, "fadvise64_64");
      CASE (SYS_fchown32, "fchown32");
      CASE (SYS_fcntl64, "fcntl64");
      CASE (SYS_fstat64, "fstat64");
      CASE (SYS_fstatat64, "fstatat64");
      CASE (SYS_fstatfs64, "fstatfs64");
      CASE (SYS_ftime, "ftime");
      CASE (SYS_ftruncate64, "ftruncate64");
      CASE (SYS_getegid32, "getegid32");
      CASE (SYS_geteuid32, "geteuid32");
      CASE (SYS_getgid32, "getgid32");
      CASE (SYS_getgroups32, "getgroups32");
      CASE (SYS_getresgid32, "getresgid32");
      CASE (SYS_getresuid32, "getresuid32");
      CASE (SYS_getuid32, "getuid32");
      CASE (SYS_gtty, "gtty");
      CASE (SYS_idle, "idle");
      CASE (SYS_ipc "ipc");
      CASE (SYS_lchown32, "lchown32");
      CASE (SYS_lock, "lock");
      CASE (SYS_lstat64, "lstat64");
      CASE (SYS_madvise1, "madvise1");
      CASE (SYS_mmap2, "mmap2");
      CASE (SYS_mpx, "mpx");
      CASE (SYS_nice, "nice");
      CASE (SYS_oldfstat, "oldfstat");
      CASE (SYS_oldlstat, "oldlstat");
      CASE (SYS_oldolduname, "oldolduname");
      CASE (SYS_oldstat, "oldstat");
      CASE (SYS_olduname, "olduname");
      CASE (SYS_prof, "prof");
      CASE (SYS_profil, "profil");
      CASE (SYS_readdir, "readdir");
      CASE (SYS_sendfile64, "sendfile64");
      CASE (SYS_setfsgid32, "setfsgid32");
      CASE (SYS_setfsuid32, "setfsuid32");
      CASE (SYS_setgid32, "setgid32");
      CASE (SYS_setgroups32, "setgroups32");
      CASE (SYS_setregid32, "setregid32");
      CASE (SYS_setresgid32, "setresgid32");
      CASE (SYS_setresuid32, "setresuid32");
      CASE (SYS_setreuid32, "setreuid32");
      CASE (SYS_setuid32, "setuid32");
      CASE (SYS_sgetmask, "sgetmask");
      CASE (SYS_sigaction, "sigaction");
      CASE (SYS_signal, "signal");
      CASE (SYS_sigpending, "sigpending");
      CASE (SYS_sigprocmask, "sigprocmask");
      CASE (SYS_sigreturn, "sigreturn");
      CASE (SYS_sigsuspend, "sigsuspend");
      CASE (SYS_socketcall, "socketcall");
      CASE (SYS_ssetmask, "ssetmask");
      CASE (SYS_stat64, "stat64");
      CASE (SYS_statfs64, "statfs64");
      CASE (SYS_stime, "stime");
      CASE (SYS_stty, "stty");
      CASE (SYS_truncate64, "truncate64");
      CASE (SYS_ugetrlimit, "ugetrlimit");
      CASE (SYS_ulimit, "ulimit");
      CASE (SYS_umount, "umount");
      CASE (SYS_vm86, "vm86");
      CASE (SYS_vm86old, "vm86old");
      CASE (SYS_waitpid, "waitpid");
#endif
    default:
      fprintf (stdout, "unknown syscall (%d)", syscall);
    }

  return;
}

void
rlimit_display (subprocess_t * p)
{
  /* 'argv' display */
  fprintf (stdout, "* Command line: ");
  for (int i = 0; i < p->argc; i++)
    {
      fprintf (stdout, "%s ", p->argv[i]);
    }
  fputs ("\n", stdout);

  /* 'envp' display */
  fprintf (stdout, "* Environment: ");
  int i = 0;
  while ((p->envp) && p->envp[i])
    {
      fprintf (stdout, "%s ", p->envp[i++]);
    }
  fputs ("\n", stdout);

  /* Limits display */
  if (p->limits)
    {
      if (p->limits->timeout > 0)
	fprintf (stdout, "* Time limit: %ds\n", p->limits->timeout);

      if (p->limits->memory > 0)
	fprintf (stdout, "* Memory limit: %d bytes\n", p->limits->memory);

      if (p->limits->fsize > 0)
	fprintf (stdout, "* File size limit: %d bytes\n", p->limits->fsize);

      if (p->limits->fd > 0)
	fprintf (stdout, "* Number of open files limit: %d\n", p->limits->fd);

      if (p->limits->proc > 0)
	fprintf (stdout, "* Number of processes limit: %d\n",
		 p->limits->proc);

      if (p->limits->syscalls[0] > 0)
	{
	  fprintf (stdout, "* Forbidden syscalls: ");
	  for (int i = 0; i < p->limits->syscalls[0]; i++)
	    {
	      fprintf_syscall (p->limits->syscalls[i + 1]);
	      fprintf (stdout, " ");
	    }
	  fprintf (stdout, "\n");
	}
    }

  /* PID display */
  fprintf (stdout, "* PID: %d\n", p->pid);

  /* Status display */
  switch (p->status)
    {
    case READY:
      fprintf (stdout, "* Status: Ready\n");
      break;
    case RUNNING:
      fprintf (stdout, "* Status: Running\n");
      break;
    case SLEEPING:
      fprintf (stdout, "* Status: Sleeping\n");
      break;
    case STOPPED:
      fprintf (stdout, "* Status: Stopped\n");
      break;
    case ZOMBIE:
      fprintf (stdout, "* Status: Zombie\n");
      break;
    case TERMINATED:
      fprintf (stdout, "* Status: Terminated (program end)\n");
      break;
    case KILLED:
      fprintf (stdout, "* Status: Terminated (killed)\n");
      break;
    case TIMEOUT:
      fprintf (stdout, "* Status: Terminated (timeout)\n");
      break;
    case MEMORYOUT:
      fprintf (stdout, "* Status: Terminated (memory out)\n");
      break;
    case FSIZEEXCEED:
      fprintf (stdout, "* Status: Terminated (file size exceeded quota)\n");
      break;
    case FDEXCEED:
      fprintf (stdout, "* Status: Terminated (fd exceeded quota)\n");
      break;
    case PROCEXCEED:
      fprintf (stdout,
	       "* Status: Terminated (processes number exceeded quota)\n");
      break;
    case DENIEDSYSCALL:
      fprintf (stdout, "* Status: Terminated (forbidden syscall)\n");
      break;

    default:
      fprintf (stdout, "* Status: Unknown status (%d)\n", p->status);
    }

  /* Profile of the program */
  if (p->profile)
    {
      fprintf (stdout, "* Profiling information:\n");
      fprintf (stdout, " + Real time: %.3fs\n",
	       (double) p->profile->real_time_usec / 1000000);
      fprintf (stdout, " + User time: %.3fs\n",
	       (double) p->profile->user_time_usec / 1000000);
      fprintf (stdout, " + System time: %.3fs\n",
	       (double) p->profile->sys_time_usec / 1000000);
      fprintf (stdout, " + Memory usage: %dkb\n",
	       (int) p->profile->memory_kbytes);
    }

  /* Return value display */
  fprintf (stdout, "* Return value: %d\n", p->retval);

  /* Printing the i/o */
  char buff[100];

  fprintf (stdout, "* Stdout: \n");
  if (p->stdout)
    {
      while (fgets (buff, 100, p->stdout))
	{
	  printf ("%s", buff);
	}
    }
  fputs ("\n", stdout);

  fprintf (stdout, "* Stderr: \n");
  if (p->stderr)
    {
      while (fgets (buff, 100, p->stderr))
	{
	  printf ("%s", buff);
	}
    }
  fputs ("\n", stdout);
}

/* Compute the difference between two timespecs */
static struct timespec
timespec_diff (struct timespec start, struct timespec end)
{
  struct timespec result;

  if ((end.tv_nsec - start.tv_nsec) < 0)
    {
      result.tv_sec = end.tv_sec - start.tv_sec - 1;
      result.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    }
  else
    {				/* Wrap things around (modulo arithmetics) */
      result.tv_sec = end.tv_sec - start.tv_sec;
      result.tv_nsec = end.tv_nsec - start.tv_nsec;
    }

  return result;
}

/* Watchdog to timeout the subprocess when needed */
static void *
watchdog (void *arg)
{
  subprocess_t *p = (subprocess_t *) arg;

  CHECK_ERROR ((pthread_detach (pthread_self ()) != 0),
	       "pthread_detach failed");

  sigset_t mask;
  sigemptyset (&mask);
  sigaddset (&mask, SIGCHLD);

  struct timespec timeout;
  timeout.tv_sec = p->limits->timeout;
  timeout.tv_nsec = 0;

  do
    {
      if (sigtimedwait (&mask, NULL, &timeout) == -1)
	{
	  if (errno == EINTR)
	    {
	      /* Interrupted by a signal other than SIGCHLD. */
	      continue;
	    }
	  else if (errno == EAGAIN)
	    {
	      p->status = TIMEOUT;
	      kill (p->pid, SIGKILL);
	    }
	  else
	    {
	      rlimit_error ("sigtimedwait failed");
	    }
	}
      break;
    }
  while (true);

fail:
  return NULL;
}

/* Monitoring the subprocess end and get the return value */
static void *
monitor (void *arg)
{
  subprocess_t *p = arg;

  bool syscall_enter = true;
  int status, syscall_id;
  pthread_t watchdog_pthread;
  struct timespec start_time;

  /* Profiling information */
  struct rusage usage;

  CHECK_ERROR ((pthread_detach (pthread_self ()) != 0),
	       "pthread_detach failed");

  /* Initializing the pipes () */
  int stdin_pipe[2];		/* '0' = child_read,  '1' = parent_write */
  int stdout_pipe[2];		/* '0' = parent_read, '1' = child_write */
  int stderr_pipe[2];		/* '0' = parent_read, '1' = child_write */

  CHECK_ERROR (((pipe (stdin_pipe) == -1) ||
		(pipe (stdout_pipe) == -1) ||
		(pipe (stderr_pipe) == -1)), "pipe initialization failed");

  /* We create a child process running the subprocess and we wait for
   * it to finish. If a timeout elapsed the child is killed. Waiting
   * is done using sigtimedwait(). Race condition is avoided by
   * blocking the SIGCHLD signal before fork() and storing it into the
   * buffer while starting the timer in a separate pthread. */

  /* Blocking SIGCHLD before forking */
  sigset_t mask;

  CHECK_ERROR ((sigemptyset (&mask) == -1), "sigemptyset failed");
  CHECK_ERROR ((sigaddset (&mask, SIGCHLD) == -1), "sigaddset failed");

  CHECK_ERROR ((sigprocmask (SIG_BLOCK, &mask, NULL) == -1),
	       "sigprocmask failed");

  /* Getting start time of the subprocess (profiling information) */
  if (p->profile != NULL)
    CHECK_ERROR ((clock_gettime (CLOCK_MONOTONIC, &start_time) == -1),
		 "getting start time failed");

  /* Forking the process */
  CHECK_ERROR (((p->pid = fork ()) == -1), "fork failed");

  if (p->pid == 0)	/***** Child process *****/
    {
      /* Set the limits on the process */
      if (p->limits != NULL)
	{
	  /* Setting the rlimits */
	  struct rlimit limit;

	  /* Setting a limit on the memory */
	  if (p->limits->memory > 0)
	    {
	      CHECK_ERROR ((getrlimit (RLIMIT_AS, &limit) == -1),
			   "getting memory limit failed");

	      limit.rlim_cur = p->limits->memory;

	      CHECK_ERROR ((setrlimit (RLIMIT_AS, &limit) == -1),
			   "setting memory limit failed");
	    }

	  /* Setting a limit on file size */
	  if (p->limits->fsize > 0)
	    {
	      CHECK_ERROR ((getrlimit (RLIMIT_FSIZE, &limit) == -1),
			   "getting file size limit failed");

	      limit.rlim_cur = p->limits->fsize;

	      CHECK_ERROR ((setrlimit (RLIMIT_FSIZE, &limit) == -1),
			   "setting file size limit failed");
	    }

	  /* Setting a limit on file descriptor number */
	  if (p->limits->fd > 0)
	    {
	      CHECK_ERROR ((getrlimit (RLIMIT_NOFILE, &limit) == -1),
			   "getting maximum fd number limit failed");

	      limit.rlim_cur = p->limits->fd;

	      CHECK_ERROR ((setrlimit (RLIMIT_NOFILE, &limit) == -1),
			   "setting maximum fd number limit failed");
	    }

	  /* Setting a limit on process number */
	  if (p->limits->proc > 0)
	    {
	      CHECK_ERROR ((getrlimit (RLIMIT_NPROC, &limit) == -1),
			   "getting maximum process number limit failed");

	      limit.rlim_cur = p->limits->proc;

	      CHECK_ERROR ((setrlimit (RLIMIT_NPROC, &limit) == -1),
			   "setting maximum process number limit failed");
	    }

	  /* Setting a syscall tracer on the process */
	  if (p->limits->syscalls[0] > 0)
	    {
	      CHECK_ERROR ((ptrace (PTRACE_TRACEME, 0, NULL, NULL) == -1),
			   "ptrace failed");
	    }
	}

      /* Setting i/o handlers */
      CHECK_ERROR ((close (stdin_pipe[1]) == -1), "close(stdin[1]) failed");
      CHECK_ERROR ((dup2 (stdin_pipe[0], STDIN_FILENO) == -1),
		   "dup(stdin) failed");
      CHECK_ERROR ((close (stdin_pipe[0]) == -1), "close(stdin[0]) failed");

      CHECK_ERROR ((close (stdout_pipe[0]) == -1), "close(stdout[0]) failed");
      CHECK_ERROR ((dup2 (stdout_pipe[1], STDOUT_FILENO) == -1),
		   "dup(stdout) failed");
      CHECK_ERROR ((close (stdout_pipe[1]) == -1), "close(stdout[1]) failed");

      CHECK_ERROR ((close (stderr_pipe[0]) == -1), "close(stderr[0]) failed");
      CHECK_ERROR ((dup2 (stderr_pipe[1], STDERR_FILENO) == -1),
		   "dup(stderr) failed");
      CHECK_ERROR ((close (stderr_pipe[1]) == -1), "close(stderr[1]) failed");

      /* Run the command line */
      execve (p->argv[0], p->argv, p->envp);

      /* Must never return after the execve */
      CHECK_ERROR (true, "execve failed");
    }
  else					    /***** Parent process *****/
    {
      CHECK_ERROR ((close (stdin_pipe[0]) == -1), "close(stdin[0]) failed");
      CHECK_ERROR (((p->stdin = fdopen (stdin_pipe[1], "w")) == NULL),
		   "fdopen(stdin[1]) failed");

      CHECK_ERROR ((close (stdout_pipe[1]) == -1), "close(stdout[1]) failed");
      CHECK_ERROR (((p->stdout = fdopen (stdout_pipe[0], "r")) == NULL),
		   "fdopen(stdout[0]) failed");

      CHECK_ERROR ((close (stderr_pipe[1]) == -1), "close(stderr[1]) failed");
      CHECK_ERROR (((p->stderr = fdopen (stderr_pipe[0], "r")) == NULL),
		   "fdopen(stderr[0]) failed");

      /* Running a watchdog to timeout the subprocess */
      if ((p->limits) && (p->limits->timeout > 0))
	CHECK_ERROR ((pthread_create (&watchdog_pthread, NULL, watchdog, p) !=
		      0), "watchdog creation failed");

      /* Set the status to RUNNING */
      p->status = RUNNING;

      /* Waiting for synchronization with monitored process */
      CHECK_ERROR (wait4 (p->pid, &status, 0, &usage) == -1, "wait failed");

      /* Filtering syscalls with ptrace */
      if ((p->limits != NULL) && (p->limits->syscalls[0] > 0))
	{
	  while (true)
	    {
	      struct user_regs_struct regs;

	      CHECK_ERROR ((ptrace (PTRACE_SYSCALL, p->pid, NULL, NULL) ==
			    -1), "ptrace failed");
	      CHECK_ERROR ((wait4 (p->pid, &status, 0, &usage) == -1),
			   "wait failed");

	      if (WIFEXITED (status))
		break;

	      CHECK_ERROR ((ptrace (PTRACE_GETREGS, p->pid, NULL, &regs) ==
			    -1), "ptrace failed");

	      /* Getting syscall number is architecture dependant */
#if __WORDSIZE == 64
	      syscall_id = regs.orig_rax;
#elif __WORSIZE == 32
	      syscall_id = regs.orig_eax;
#endif

	      if (syscall_enter)
		{
		  for (int i = 1; i <= p->limits->syscalls[0]; i++)
		    if (syscall_id == p->limits->syscalls[i])
		      {
			rlimit_kill (p);
			p->status = DENIEDSYSCALL;
			goto fail;
		      }
		}

	      syscall_enter ^= true;
	    }
	}

      /***** The subprocess is finished now *****/

      /* Getting end time of the subprocess (profiling information) */
      if (p->profile != NULL)
	{
	  struct timespec tmp_time, end_time;

	  CHECK_ERROR ((clock_gettime (CLOCK_MONOTONIC, &end_time) == -1),
		       "getting end time failed");
	  tmp_time = timespec_diff (start_time, end_time);

	  p->profile->real_time_usec =
	    (time_t) (tmp_time.tv_sec * 1000000 + tmp_time.tv_nsec / 1000);
	}

      /* Finding out what the status and retval are really */
      if (WIFEXITED (status))
	{			/* Exited normally */
	  p->status = TERMINATED;
	  p->retval = WEXITSTATUS (status);	/* Return value */
	}
      else if (WIFSIGNALED (status))
	{
	  p->retval = WTERMSIG (status);	/* Kill signal */

	  if (p->status < TERMINATED)
	    {
	      /* Trying to guess why by looking at errno value */
	      switch (errno)
		{
		case ETIME:
		  p->status = TIMEOUT;
		  break;

		case ENOMEM:
		  p->status = MEMORYOUT;
		  break;

		default:
		  p->status = KILLED;
		}
	    }
	  /* FIXME: It may be interesting to store the termination signal
	   * into another variable and to 'p->retval = errno' */
	}
      else if (WIFSTOPPED (status))
	{
	  p->status = STOPPED;
	  p->retval = WSTOPSIG (status);	/* Stop signal */
	}
      else if (WIFCONTINUED (status))
	{
	  p->status = RUNNING;
	  p->retval = 0;	/* Process is still running */
	}
    }

fail:
  /* Cleaning the watchdog if not already exited */
  if ((p->limits) && (p->limits->timeout > 0))
    pthread_cancel (watchdog_pthread);

  /* Cleaning and setting the profile information */
  if (p->profile != NULL)
    {
      /* User time in us */
      p->profile->user_time_usec =
	usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec;

      /* System time in us */
      p->profile->sys_time_usec =
	usage.ru_stime.tv_sec * 1000 + usage.ru_stime.tv_usec;

      /* Memory usage */
      p->profile->memory_kbytes = usage.ru_maxrss;
    }

  return NULL;
}

int
rlimit_run (subprocess_t * p)
{
  pthread_t monitor_pthread;
  int ret = RETURN_SUCCESS;

  /* Running a thread to wait for subprocess return value */
  CHECK_ERROR ((pthread_create (&monitor_pthread, NULL, monitor, p) != 0),
	       "monitor creation failed");

  if (false)
    {
    fail:
      ret = RETURN_FAILURE;
    }

  return ret;
}

int
rlimit_kill (subprocess_t * p)
{
  int ret;

  if ((ret = kill (p->pid, SIGKILL)) == -1)
    rlimit_error ("kill failed");

  return ret;
}

int
rlimit_suspend (subprocess_t * p)
{
  int ret;

  if ((ret = kill (p->pid, SIGSTOP)) == -1)
    rlimit_error ("suspend failed");

  return ret;
}

int
rlimit_resume (subprocess_t * p)
{
  int ret;

  if ((ret = kill (p->pid, SIGCONT)) == -1)
    rlimit_error ("resume failed");

  return ret;
}

int
rlimit_poll (subprocess_t * p)
{
  return p->status;
}

int
rlimit_wait (subprocess_t * p)
{
  struct timespec tick;

  tick.tv_sec = 0;
  tick.tv_nsec = 100;

  while (p->status < TERMINATED)
    nanosleep (&tick, NULL);

  return p->retval;
}

int
rlimit_signal (int signal, subprocess_t * p)
{
  int ret;

  if ((ret = kill (p->pid, signal)) == -1)
    rlimit_error ("signal failed");

  return ret;
}


/***** Setters and getters *****/

void
rlimit_set_time_limit (int timeout, subprocess_t * p)
{
  if (p->limits == NULL)
    p->limits = limits_new ();

  if (p->limits != NULL)
    p->limits->timeout = timeout;
  else
    rlimit_error ("setting time limit failed");
}

int
rlimit_get_time_limit (subprocess_t * p)
{
  int time = 0;

  if (p->limits != NULL)
    time = p->limits->timeout;

  return time;
}

void
rlimit_set_memory_limit (int memory, subprocess_t * p)
{
  if (p->limits == NULL)
    p->limits = limits_new ();

  if (p->limits != NULL)
    p->limits->memory = memory;
  else
    rlimit_error ("setting memory limit failed");
}

int
rlimit_get_memory_limit (subprocess_t * p)
{
  int memory = 0;

  if (p->limits != NULL)
    memory = p->limits->memory;

  return memory;
}

void
rlimit_set_fsize_limit (int fsize, subprocess_t * p)
{
  if (p->limits == NULL)
    p->limits = limits_new ();

  if (p->limits != NULL)
    p->limits->fsize = fsize;
  else
    rlimit_error ("setting file size limit failed");
}

int
rlimit_get_fsize_limit (subprocess_t * p)
{
  int fsize = 0;

  if (p->limits != NULL)
    fsize = p->limits->fsize;

  return fsize;
}

void
rlimit_set_fd_limit (int fd, subprocess_t * p)
{
  if (p->limits == NULL)
    p->limits = limits_new ();

  if (p->limits != NULL)
    p->limits->fd = fd;
  else
    rlimit_error ("setting fd limit failed");
}

int
rlimit_get_fd_limit (subprocess_t * p)
{
  int fd = 0;

  if (p->limits != NULL)
    fd = p->limits->fd;

  return fd;
}

void
rlimit_set_proc_limit (int proc, subprocess_t * p)
{
  if (p->limits == NULL)
    p->limits = limits_new ();

  if (p->limits != NULL)
    p->limits->proc = proc;
  else
    rlimit_error ("setting processes limit failed");
}

int
rlimit_get_proc_limit (subprocess_t * p)
{
  int proc = 0;

  if (p->limits != NULL)
    proc = p->limits->proc;

  return proc;
}

void
rlimit_disable_syscall (int syscall, subprocess_t * p)
{
  if (p->limits == NULL)
    p->limits = limits_new ();

  if (p->limits != NULL)
    {
      p->limits->syscalls[0] += 1;
      p->limits->syscalls = realloc (p->limits->syscalls,
				     (p->limits->syscalls[0] + 1)
				     * sizeof p->limits->syscalls[0]);

      /* Check if realloc failed or not */
      if (p->limits->syscalls == NULL)
	goto fail;

      /* Adding the syscall to the list */
      p->limits->syscalls[(p->limits->syscalls[0])] = syscall;
    }
  else
    {
    fail:
      rlimit_error ("disabling syscall failed");
    }
}

int *
rlimit_get_disabled_syscalls (subprocess_t * p)
{
  int *syscalls = NULL;

  if (p->limits != NULL)
    syscalls = p->limits->syscalls;

  return syscalls;
}


/***** Profile information *****/
int
rlimit_profile_init (subprocess_t * p)
{
  int ret = RETURN_SUCCESS;

  if (p->profile == NULL)
    p->profile = malloc (sizeof (profile_t));

  if (p->profile == NULL)
    {
      perror ("profiling failed");
      goto fail;
    }

  p->profile->real_time_usec = 0;
  p->profile->user_time_usec = 0;
  p->profile->sys_time_usec = 0;
  p->profile->memory_kbytes = 0;

  if (false)
    {
    fail:
      ret = RETURN_FAILURE;
    }

  return ret;
}

time_t
rlimit_get_real_time (subprocess_t * p)
{
  int ret = 0;

  if (p->profile)
    ret = p->profile->real_time_usec;

  return ret;
}

time_t
rlimit_get_user_time (subprocess_t * p)
{
  int ret = 0;

  if (p->profile)
    ret = p->profile->user_time_usec;

  return ret;
}

time_t
rlimit_get_sys_time (subprocess_t * p)
{
  int ret = 0;

  if (p->profile)
    ret = p->profile->sys_time_usec;

  return ret;
}

size_t
rlimit_get_memory (subprocess_t * p)
{
  int ret = 0;

  if (p->profile)
    ret = p->profile->memory_kbytes;

  return ret;
}
