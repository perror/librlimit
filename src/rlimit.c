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

#define _BSD_SOURCE		/* needed by wait4() */
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <pthread.h>
#include <regex.h>
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

static char *error = "librlimit: error";
static char *warning = "librlimit: warning";

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
      if ((s =
	   malloc (snprintf (NULL, 0, "%s: %s", warning, msg) + 1)) == NULL)
	s = error;
      else
	sprintf (s, "%s: %s", warning, msg);
    }
  perror (s);
}

subprocess_t *
rlimit_subprocess_create (int argc, char **argv, char **envp)
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
      p = NULL;
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
	  p = NULL;
	  CHECK_ERROR (true, "subprocess allocation failed");
	}

      memcpy (p->argv[i], argv[i], strlen(argv[i]) + 1);
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
	  for (int i = 0; p->argv[i] != NULL; i++)
	    free (p->argv[i]);
	  free (p->argv);
	  free (p);
	  p = NULL;
	  CHECK_ERROR (true, "subprocess allocation failed");
	}

      for (int i = 0; i < envp_size; i++)
	{
	  p->envp[i] = malloc (strlen (envp[i] + 1) * sizeof (char));
	  /* Handling 'out of memory' */
	  if (p->envp[i] == NULL)
	    {
	      for (int i = 0; p->argv[i] != NULL; i++)
		free (p->argv[i]);
	      free (p->argv);
	      while (i >= 0)
		free (p->envp[i--]);
	      free (p->envp);
	      free (p);
	      p = NULL;
	      CHECK_ERROR (true, "subprocess allocation failed");
	    }

	  memcpy (p->envp[i], envp[i], strlen(envp[i]) + 1);
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

  p->stdin_buffer = NULL;
  p->stdout_buffer = NULL;
  p->stderr_buffer = NULL;

  /* Initializing the limits and profile to default */

  p->real_time_usec = 0;
  p->user_time_usec = 0;
  p->sys_time_usec = 0;
  p->memory_kbytes = 0;

  p->limits = NULL;

  /* Initializing the private fields */
  p->expect_stdout  = 0;
  p->expect_stderr = 0;

  p->monitor = malloc (sizeof (pthread_t));
  CHECK_ERROR ((p->monitor == NULL), "p->monitor allocation failed");

  pthread_mutex_init (&(p->write_mutex), NULL);

fail:
  return p;
}

static limits_t *
limits_new (void)
{
  limits_t *limits = malloc (sizeof (limits_t));
  CHECK_ERROR ((limits == NULL), "limits allocation failed");

  /* Default initialization of limits */
  limits->timeout = 0;
  limits->memory = 0;
  limits->fsize = 0;
  limits->fd = 0;
  limits->proc = 0;

  limits->syscalls = malloc (sizeof (int));
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
rlimit_subprocess_delete (subprocess_t * p)
{
  /* Discarding case 'p == NULL' */
  if (!p)
    return;

  /* Handling still non-dead subprocesses */
  /* FIXME: What to do if subprocess is still running ? */
  if (p->status < TERMINATED)
    {
      rlimit_warning ("subprocess was still running");
      rlimit_subprocess_kill (p);
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
  fclose (p->stdin);
  fclose (p->stdout);
  fclose (p->stderr);

  /* Freeing buffers */
  free (p->stdin_buffer);
  free (p->stdout_buffer);
  free (p->stderr_buffer);

  /* Freeing the monitor and write mutex */
  free (p->monitor);
  pthread_mutex_destroy (&(p->write_mutex));

  /* Freeing the limits_t */
  if (p->limits)
    limits_delete (p->limits);

  /* Freeing the subprocess */
  free (p);
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

/* IO monitor to watch the stdin, stdout and stderr file descriptors */
static void *
io_monitor (void *arg)
{
  subprocess_t *p = arg;

  int nfds;
  fd_set rfds, wfds;

  int stdout_fd = fileno (p->stdout);
  int stderr_fd = fileno (p->stderr);
  int stdin_fd = fileno (p->stdin);

  size_t stdout_size = 0;
  size_t stdout_current = 0;
  size_t stderr_size = 0;
  size_t stderr_current = 0;

  while (true)
    {
      FD_ZERO (&rfds);
      FD_SET (stdout_fd, &rfds);
      FD_SET (stderr_fd, &rfds);

      FD_ZERO (&wfds);
      FD_SET (stdin_fd, &wfds);

      nfds = (stdout_fd > stderr_fd) ? stdout_fd : stderr_fd;
      nfds = (stdin_fd > nfds) ? stdin_fd : nfds;

      CHECK_ERROR ((select (nfds + 1, &rfds, &wfds, NULL, NULL) == -1),
		   "select() failed");

      size_t count;
      char buffer_stdout[256], buffer_stderr[256];

      if (FD_ISSET (stdout_fd, &rfds))
	{
	  CHECK_ERROR (((int) (count = read (stdout_fd,
					     buffer_stdout,
					     sizeof (buffer_stdout))) == -1),
		       "read(stdout) failed");

	  /* Expand memory if not enough space left */
	  if ((stdout_current + count + 1) > stdout_size)
	    {
	      stdout_size = (stdout_current + count + 1) * 2;

	      p->stdout_buffer = realloc (p->stdout_buffer, stdout_size);
	      CHECK_ERROR ((p->stdout_buffer == NULL), "stdout read failed");
	    }

	  memcpy(&(p->stdout_buffer[stdout_current]), buffer_stdout, count);
	  stdout_current += count;
	  p->stdout_buffer[stdout_current] = '\0';
	}

      if (FD_ISSET (stderr_fd, &rfds))
	{
	  CHECK_ERROR (((int) (count = read (stderr_fd,
					     buffer_stderr,
					     sizeof (buffer_stderr))) == -1),
		       "read(stderr) failed");

	  if ((stderr_current + count + 1) > stderr_size)
	    {
	      stderr_size +=
		((stderr_current + count + 1 - stderr_size) / 1024 + 1) * 1024;

	      p->stderr_buffer = realloc (p->stderr_buffer, stderr_size);
	      CHECK_ERROR ((p->stderr_buffer == NULL), "stderr read failed");
	    }

	  memcpy(&(p->stderr_buffer[stderr_current]), buffer_stderr, count);
	  stderr_current += count;
	  p->stderr_buffer[stderr_current] = '\0';
	}

      if ((FD_ISSET (stdin_fd, &wfds)) && (p->stdin_buffer != NULL))
	{
	  size_t size = strlen (p->stdin_buffer);

	  count = write (stdin_fd, p->stdin_buffer, size);

	  if (((int) count == -1) && (count != size))
	    {
	      rlimit_error ("write(stdin) failed");
	      goto fail;
	    }

	  free (p->stdin_buffer);
	  p->stdin_buffer = NULL;
	}
    }

fail:
  return NULL;
}

/* Watchdog to timeout the subprocess when needed */
static void *
watchdog (void *arg)
{
  subprocess_t *p = arg;
  sigset_t mask;
  sigemptyset (&mask);
  sigaddset (&mask, SIGCHLD);

  struct timespec timeout;
  timeout.tv_sec = p->limits->timeout;
  timeout.tv_nsec = 0;

  while (true)
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
	      if (p->status < TERMINATED)
		{
		  p->status = TIMEOUT;
		  kill (p->pid, SIGKILL);
		}
	    }
	  else
	    {
	      rlimit_error ("sigtimedwait failed");
	    }
	}
      break;
    }

  return NULL;
}

/* Monitor for the child process */
static int
child_monitor (subprocess_t * p,
	       int stdin_pipe[2], int stdout_pipe[2], int stderr_pipe[2])
{
  int ret = RETURN_SUCCESS;

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
  CHECK_ERROR((execve (p->argv[0], p->argv, p->envp) == -1),
	      "execve failed");

  if (false)
  fail:
    ret = RETURN_FAILURE;

  return ret;
}

static int
syscall_filter (subprocess_t * p, int *status, struct rusage *usage)
{
  int syscall_id;
  bool syscall_enter = true;
  int ret = RETURN_SUCCESS;

  while (true)
    {
      struct user_regs_struct regs;

      CHECK_ERROR ((ptrace (PTRACE_SYSCALL, p->pid, NULL, NULL) == -1),
		   "ptrace failed");
      CHECK_ERROR ((wait4 (p->pid, status, 0, usage) == -1), "wait failed");

      if (WIFEXITED (*status) || WIFSIGNALED (*status) || WCOREDUMP (*status))
	break;

      CHECK_ERROR ((ptrace (PTRACE_GETREGS, p->pid, NULL, &regs) == -1),
		   "ptrace failed");

      /* Getting syscall number is architecture dependant */
#if __WORDSIZE == 64
      syscall_id = regs.orig_rax;
#elif __WORDSIZE == 32
      syscall_id = regs.orig_eax;
#endif

      if (syscall_enter)
	{
	  for (int i = 1; i <= p->limits->syscalls[0]; i++)
	    if (syscall_id == p->limits->syscalls[i])
	      {
		p->status = DENIEDSYSCALL;
		rlimit_subprocess_kill (p);
		goto fail;
	      }
	}

      syscall_enter ^= true;
    }

  if (false)
  fail:
    ret = RETURN_FAILURE;

  return ret;
}

/* Monitoring the subprocess end and get the return value */
static void *
monitor (void *arg)
{
  subprocess_t *p = arg;
  struct timespec start_time;

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
  CHECK_ERROR ((clock_gettime (CLOCK_MONOTONIC, &start_time) == -1),
	       "getting start time failed");

  /* Forking the process */
  CHECK_ERROR (((p->pid = fork ()) == -1), "fork failed");

  if (p->pid == 0)	/***** Child process *****/
    {
      /* TODO: What if the child_monitor fails miserably ? It should
       * be signaled in the parent stderr and not in the child
       * stderr. */
      CHECK_ERROR ((child_monitor (p,
				   stdin_pipe,
				   stdout_pipe,
				   stderr_pipe) == RETURN_FAILURE),
		   "child monitor failed");
    }
  else			    /***** Parent process *****/
    {
      int status;
      pthread_t watchdog_pthread, io_pthread;
      struct rusage usage;

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

      /* Running the io monitor to watch stdout and stderr */
      CHECK_ERROR ((pthread_create (&io_pthread, NULL, io_monitor, p) != 0),
		   "io_monitor creation failed");

      p->status = RUNNING;

      /* Waiting for synchronization with monitored process */
      CHECK_ERROR (wait4 (p->pid, &status, 0, &usage) == -1, "wait failed");

      /* Filtering syscalls with ptrace */
      if ((p->limits != NULL) && (p->limits->syscalls[0] > 0))
	{
	  if (syscall_filter (p, &status, &usage) == RETURN_FAILURE)
	    goto fail;
	}

      /***** The subprocess is finished now *****/

      /* Getting end time of the subprocess (profiling information) */
      struct timespec tmp_time, end_time;

      CHECK_ERROR ((clock_gettime (CLOCK_MONOTONIC, &end_time) == -1),
		   "getting end time failed");
      tmp_time = timespec_diff (start_time, end_time);

      p->real_time_usec =
	(time_t) (tmp_time.tv_sec * 1000000 + tmp_time.tv_nsec / 1000);

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
	      switch (WTERMSIG (status))
		{
		case SIGSEGV:
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

    fail:
      /* Cleaning the io_monitor */
      pthread_cancel (io_pthread);

      /* Cleaning the watchdog if not already exited */
      if ((p->limits) && (p->limits->timeout > 0))
	pthread_cancel (watchdog_pthread);

      /* Cleaning and setting the profile information */
      /* User time in us */
      p->user_time_usec =
	usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec;

      /* System time in us */
      p->sys_time_usec =
	usage.ru_stime.tv_sec * 1000 + usage.ru_stime.tv_usec;

      /* Memory usage */
      p->memory_kbytes = usage.ru_maxrss;
    }

  return NULL;
}

int
rlimit_subprocess_run (subprocess_t * p)
{
  int ret = RETURN_SUCCESS;

  /* Running a monitor thread to wait for subprocess return value */
  CHECK_ERROR ((pthread_create (p->monitor, NULL, monitor, p) != 0),
	       "monitor creation failed");

  if (false)
  fail:
    ret = RETURN_FAILURE;

  return ret;
}

int
rlimit_subprocess_kill (subprocess_t * p)
{
  int ret;

  if ((ret = kill (p->pid, SIGKILL)) == -1)
    rlimit_error ("kill failed");

  return ret;
}

int
rlimit_subprocess_suspend (subprocess_t * p)
{
  int ret;

  if ((ret = kill (p->pid, SIGSTOP)) == -1)
    rlimit_error ("suspend failed");

  return ret;
}

int
rlimit_subprocess_resume (subprocess_t * p)
{
  int ret;

  if ((ret = kill (p->pid, SIGCONT)) == -1)
    rlimit_error ("resume failed");

  return ret;
}

void
rlimit_write_stdin (subprocess_t * p, char * msg)
{
  ssize_t size = strlen (msg);

  struct timespec tick;
  tick.tv_sec = 0;
  tick.tv_nsec = 100;

  pthread_mutex_lock (&(p->write_mutex));

  char * tmp = malloc ((size + 1) * sizeof (char));
  CHECK_ERROR ((tmp == NULL), "write failed");

  memcpy (tmp, msg, size);
  tmp[size] = '\0';

  p->stdin_buffer = tmp;

  /* Wait until msg has been read or the process is finished */
  while ((p->stdin_buffer != NULL) && (p->status < TERMINATED))
    nanosleep (&tick, NULL);

 fail:
  pthread_mutex_unlock (&(p->write_mutex));
}

char *
rlimit_read_stdout (subprocess_t * p)
{
  return (p->stdout_buffer);
}

char *
rlimit_read_stderr (subprocess_t * p)
{
  return (p->stderr_buffer);
}

bool rlimit_expect (subprocess_t * p, char * pattern, int timeout)
{
  regex_t regex;
  bool result = false;
  int new_expect_stdout = 0;
  int new_expect_stderr = 0;
  struct timespec start_time, current_time;

  struct timespec tick;
  tick.tv_sec = 0;
  tick.tv_nsec = 100;

  /* Getting start time */
  CHECK_ERROR ((clock_gettime (CLOCK_MONOTONIC, &start_time) == -1),
	       "getting start time failed");

  /* Getting the regular expression compiled */
  int error = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
  char * str_stdout = &(p->stdout_buffer[p->expect_stdout]);
  char * str_stderr = &(p->stderr_buffer[p->expect_stderr]);

  /* Dealing with errors (if any) */
  if (error != 0)
    {
      char msgbuf[128];
      regerror(error, &regex, msgbuf, sizeof(msgbuf));

      CHECK_ERROR(true, msgbuf);
    }

  /* Main loop: Wait until the pattern appears or the timeout is hit */
  do
    {
      /* Wait a tick */
      nanosleep (&tick, NULL);

      if (!p->stdout_buffer)
	continue;

      /* Get future expect_stdout/stderr (avoiding increase after search) */
      new_expect_stdout = strlen (p->stdout_buffer);
      new_expect_stderr = strlen (p->stderr_buffer);

      /* Searching for pattern */
      if (regexec(&regex, str_stdout, (size_t) 0, NULL, 0) == 0)
	{
	  result = true;
	  break;
	}

      if (!p->stderr_buffer)
	continue;

      /* Get future expect_stdout (avoiding increase after search) */
      new_expect_stderr = strlen (p->stderr_buffer);

      /* Searching for pattern */
      if (regexec(&regex, str_stderr, (size_t) 0, NULL, 0) == 0)
	{
	  result = true;
	  break;
	}

      /* Getting current time */
      CHECK_ERROR ((clock_gettime (CLOCK_MONOTONIC, &current_time) == -1),
	       "getting current time failed");
    }
  while ((current_time.tv_sec - start_time.tv_sec) < timeout);

  /* Setting expect_stdout/stderr to the new value */
  p->expect_stdout = new_expect_stdout;
  p->expect_stderr = new_expect_stderr;

 fail:
  regfree(&regex);

  return result;
}

bool rlimit_expect_stdout (subprocess_t * p, char * pattern, int timeout)
{
  regex_t regex;
  bool result = false;
  int new_expect_stdout = 0;
  struct timespec start_time, current_time;

  struct timespec tick;
  tick.tv_sec = 0;
  tick.tv_nsec = 100;

  /* Getting start time */
  CHECK_ERROR ((clock_gettime (CLOCK_MONOTONIC, &start_time) == -1),
	       "getting start time failed");

  /* Getting the regular expression compiled */
  int error = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
  char * str = &(p->stdout_buffer[p->expect_stdout]);

  /* Dealing with errors (if any) */
  if (error != 0)
    {
      char msgbuf[128];
      regerror(error, &regex, msgbuf, sizeof(msgbuf));

      CHECK_ERROR(true, msgbuf);
    }

  /* Main loop: Wait until the pattern appears or the timeout is hit */
  do
    {
      /* Wait a tick */
      nanosleep (&tick, NULL);

      if (!p->stdout_buffer)
	continue;

      /* Get future expect_stdout (avoiding increase after search) */
      new_expect_stdout = strlen (p->stdout_buffer);

      /* Searching for pattern */
      if (regexec(&regex, str, (size_t) 0, NULL, 0) == 0)
	{
	  result = true;
	  break;
	}

      /* Getting current time */
      CHECK_ERROR ((clock_gettime (CLOCK_MONOTONIC, &current_time) == -1),
	       "getting current time failed");
    }
  while (((current_time.tv_sec - start_time.tv_sec) < timeout) &&
	 (p->status < TERMINATED));

  /* Setting expect_stdout to the new value */
  p->expect_stdout = new_expect_stdout;

 fail:
  regfree(&regex);

  return result;
}

bool rlimit_expect_stderr (subprocess_t * p, char * pattern, int timeout)
{
  regex_t regex;
  bool result = false;
  int new_expect_stderr = 0;
  struct timespec start_time, current_time;

  struct timespec tick;
  tick.tv_sec = 0;
  tick.tv_nsec = 100;

  /* Getting start time */
  CHECK_ERROR ((clock_gettime (CLOCK_MONOTONIC, &start_time) == -1),
	       "getting start time failed");

  /* Getting the regular expression compiled */
  int error = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
  char * str = &(p->stderr_buffer[p->expect_stderr]);

  /* Dealing with errors (if any) */
  if (error != 0)
    {
      char msgbuf[128];
      regerror(error, &regex, msgbuf, sizeof(msgbuf));

      CHECK_ERROR(true, msgbuf);
    }

  /* Main loop: Wait until the pattern appears or the timeout is hit */
  do
    {
      /* Wait a tick */
      nanosleep (&tick, NULL);

      if (!p->stderr_buffer)
	continue;

      /* Get future expect_stderr (avoiding increase after search) */
      new_expect_stderr = strlen (p->stderr_buffer);

      /* Searching for pattern */
      if (regexec(&regex, str, (size_t) 0, NULL, 0) == 0)
	{
	  result = true;
	  break;
	}

      /* Getting current time */
      CHECK_ERROR ((clock_gettime (CLOCK_MONOTONIC, &current_time) == -1),
	       "getting current time failed");
    }
  while ((current_time.tv_sec - start_time.tv_sec) < timeout);

  /* Setting expect_stderr to the new value */
  p->expect_stderr = new_expect_stderr;

 fail:
  regfree(&regex);

  return result;
}

int
rlimit_subprocess_poll (subprocess_t * p)
{
  return p->status;
}

int
rlimit_subprocess_wait (subprocess_t * p)
{
  if (pthread_join (*(p->monitor), NULL) != 0)
    perror ("pthread_join to monitor failed");

  return p->retval;
}

int
rlimit_subprocess_signal (subprocess_t * p, int signal)
{
  int ret;

  if ((ret = kill (p->pid, signal)) == -1)
    rlimit_error ("signal failed");

  return ret;
}


/***** Setters and getters *****/

void
rlimit_set_time_limit (subprocess_t * p, int timeout)
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
rlimit_set_memory_limit (subprocess_t * p, int memory)
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
rlimit_set_fsize_limit (subprocess_t * p, int fsize)
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
rlimit_set_fd_limit (subprocess_t * p, int fd)
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
rlimit_set_proc_limit (subprocess_t * p, int proc)
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
rlimit_disable_syscall (subprocess_t * p, int syscall)
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
time_t
rlimit_get_real_time (subprocess_t * p)
{
  return p->real_time_usec;
}

time_t
rlimit_get_user_time (subprocess_t * p)
{
  return p->user_time_usec;
}

time_t
rlimit_get_sys_time (subprocess_t * p)
{
  return p->sys_time_usec;
}

size_t
rlimit_get_memory (subprocess_t * p)
{
  return p->memory_kbytes;
}
