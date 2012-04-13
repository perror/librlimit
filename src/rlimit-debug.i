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
      CASE (SYS_ipc, "ipc");
      CASE (SYS_lchown32, "lchown32");
      CASE (SYS_lock, "lock");
      CASE (SYS_lstat64, "lstat64");
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
rlimit_subprocess_print (subprocess_t * p)
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
