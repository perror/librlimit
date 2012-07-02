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
 *  * 03/26/2012 (Emmanuel Fleury): First public release
 */

#ifndef RLIMIT_H
#define RLIMIT_H

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include <pthread.h>

#include <sys/types.h>
#include <sys/syscall.h>

/* Subprocess status */
#define READY      0		/* Ready to start */
#define RUNNING    1		/* Running */
#define SLEEPING   2		/* Currently not scheduled */
#define STOPPED    3		/* Interrupted and waiting */
#define ZOMBIE     4		/* Waiting for his parent before terminating */
#define TERMINATED 5		/* Normally finished */

/* Terminated by hitting a limit */
#define KILLED         6	/* Execution interrupted by a signal */
#define TIMEOUT        7	/* Timeout */
#define MEMORYOUT      8	/* Out of memory */
#define FSIZEEXCEED    9	/* File size exceeded */
#define FDEXCEED      10	/* Number of file descriptors exceeded */
#define PROCEXCEED    11	/* Number of processes exceeded */
#define DENIEDSYSCALL 12	/* Use of forbidden syscall */

/* Limit over the subprocess */
typedef struct limits
{
  int timeout;			/* Timeout (in seconds) */
  int memory;			/* Maximum memory size (in bytes) */
  int fsize;			/* Maximum file size (in bytes) */
  int fd;			/* Maximum number of open file descriptor */
  int proc;			/* Maximum number of processes */
  int *syscalls;		/* Forbiden syscalls (syscalls[0] is
				   the number of forbiden syscalls and
				   syscalls[i] (i>0) are the ids' of
				   the forbiden syscalls). */
} limits_t;

typedef struct subprocess
{
  int argc;			/* Arguments' number */
  char **argv;			/* Command line */
  char **envp;			/* Environment variables */

  pid_t pid;			/* Subprocess ID */
  int status;			/* Subprocess status */
  int retval;			/* Subprocess return value */

  FILE *stdin;			/* Subprocess stdin handler */
  FILE *stdout;			/* Subprocess stdout handler */
  FILE *stderr;			/* Subprocess stderr handler */

  char *stdin_buffer;		/* Buffer storing stdin input */
  char *stdout_buffer;		/* Buffer storing stdout output */
  char *stderr_buffer;		/* Buffer storing stderr output */

  time_t real_time_usec;	/* Real time (in micro-seconds) */
  time_t user_time_usec;	/* User time (in micro-seconds) */
  time_t sys_time_usec;		/* System time (in micro-seconds) */
  size_t memory_kbytes;		/* Maximum global memory used by the childs */

  limits_t *limits;		/* Limits on the subprocess */

  /* private fields */
  int expect_stdout;            /* Position of the expect cursor in stdout */
  int expect_stderr;            /* Position of the expect cursor in stderr */
  pthread_t *monitor;		/* Reference to the monitor thread */
  pthread_mutex_t write_mutex;	/* Mutex locking the writing on stdin */
} subprocess_t;

/* Handling subprocesses */
/* ********************* */

/* Create/delete a subprocess */
subprocess_t *rlimit_subprocess_create (int argc, char **argv, char **envp);
void rlimit_subprocess_delete (subprocess_t * p);

#ifdef DEBUG
/* Display a subprocess for debug */
void rlimit_subprocess_print (subprocess_t * p);
#endif /* DEBUG */

/* run, kill, suspend and resume a subprocess.
 * Returns '0' if everything went fine, '-1' otherwise. */
int rlimit_subprocess_run (subprocess_t * p);
int rlimit_subprocess_kill (subprocess_t * p);
int rlimit_subprocess_suspend (subprocess_t * p);
int rlimit_subprocess_resume (subprocess_t * p);

/* Handling input/output to a subprocess */
void rlimit_write_stdin (subprocess_t * p, char * msg);
char *rlimit_read_stdout (subprocess_t * p);
char *rlimit_read_stderr (subprocess_t * p);

/* Look for 'pattern' in recent output of the subprocess (see: regex.h) */
bool rlimit_expect (subprocess_t * p, char * pattern, int timeout);
bool rlimit_expect_stdout (subprocess_t * p, char * pattern, int timeout);
bool rlimit_expect_stderr (subprocess_t * p, char * pattern, int timeout);

/* Check if the subprocess is terminated ('1' if terminated, '0' otherwise). */
int rlimit_subprocess_poll (subprocess_t * p);

/* Wait for child process to terminate and return p->retval */
int rlimit_subprocess_wait (subprocess_t * p);

/* Send a signal to the subprocess */
int rlimit_subprocess_signal (subprocess_t * p, int signal);

/* Setting/getting the subprocess limitation (default: 0 (unlimited)) */
/* ****************************************************************** */

/* Set/get the timeout (in seconds) */
void rlimit_set_time_limit (subprocess_t * p, int timeout);
int rlimit_get_time_limit (subprocess_t * p);

/* Set/get the maximum memory consumption (in bytes) */
void rlimit_set_memory_limit (subprocess_t * p, int memory);
int rlimit_get_memory_limit (subprocess_t * p);

/* Set/get the maximum file size (in bytes) */
void rlimit_set_fsize_limit (subprocess_t * p, int fsize);
int rlimit_get_fsize_limit (subprocess_t * p);

/* Set/get the maximum number of file descriptors used at once */
void rlimit_set_fd_limit (subprocess_t * p, int fd);
int rlimit_get_fd_limit (subprocess_t * p);

/* Set/get the maximum number of processes used at once */
void rlimit_set_proc_limit (subprocess_t * p, int proc);
int rlimit_get_proc_limit (subprocess_t * p);

/* Disable specific syscalls */
void rlimit_disable_syscall (subprocess_t * p, int syscall);
int *rlimit_get_disabled_syscalls (subprocess_t * p);

/* Getting subprocess profiling information */
/* **************************************** */
/* Time spend in total by the process (idle time included) */
time_t rlimit_get_real_time_profile (subprocess_t * p);
/* Time spend in user-land by the subprocess and its childs */
time_t rlimit_get_user_time_profile (subprocess_t * p);
/* Time spend in kernel-land by the subprocess and its childs */
time_t rlimit_get_sys_time_profile (subprocess_t * p);

/* Maximum amount of memory used */
size_t rlimit_get_memory_profile (subprocess_t * p);

#endif /* RLIMIT_H */
