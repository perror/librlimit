'''
Subprocess is a Python class to control and profile processes in a
convenient and flexible manner. It is primarily designed for black-box
testing over untrusted code in an educational context. It is also
fully based on the C library librlimit.
'''

try:
    from ctypes import *
except ImportError as err:
    raise ImportError (str(err) + '''

A needed module for Subprocess was not found.
You should install it and try again.
''')

try:
    rlimit = cdll.LoadLibrary("librlimit.so")
    libc = cdll.LoadLibrary("libc.so.6")
except OSError as err:
    raise OSError (str(err) + '''

librlimit.so cannot be found on your system.
You should install it and/or set up properly your system.
''')

class SUBPROCESS(Structure):
    _fields_ = [("argc", c_int),
                ("argv", POINTER(c_char_p)),
                ("envp", POINTER(c_char_p)),
                ("pid", c_int),
                ("status", c_int),
                ("retval", c_int),
                ("stdin", c_void_p),
                ("stdout", c_void_p),
                ("stderr", c_void_p),
                ("stdin_buffer", c_char_p),
                ("stdout_buffer", c_char_p),
                ("stderr_buffer", c_char_p),
                ("limits", c_void_p),
                ("profile", c_void_p),
                ("expect_stdout", c_int),
                ("expect_stderr", c_int),
                ("monitor", c_void_p),
                ("write_mutex", c_int)]

class Subprocess(object):
    '''Subprocess class is intended to provide a basic control over
    untrusted subprocesses.

    This class provide a simple interface to run subprocesses in a
    non-blocking manner and, yet, getting their results. It also
    allows to have a control over the execution time, the maximum
    amount of memory it can use, and also provide basic profiling
    information (time, memory) about the subprocess.
    '''

    def __init__(self, cmd, env=None):
        '''Default constructor for a subprocess.

        This constructor requires at least the command ('cmd') to be
        executed and might requires to set the proper environment
        variables ('env') in order to run it properly.
        '''
        self.cmd = cmd
        self.env = env

        # Translating cmd/env into C arguments through ctypes
        argv_type = c_char_p * len(cmd)
        argv = argv_type(*cmd)
        argc = c_int(len(cmd))

        if (env == None):
            envp = None
        else:
            envp = argv_type(*env)

        # Getting the subprocess pointer
        rlimit.rlimit_subprocess_create.restype = POINTER(SUBPROCESS)

        self.subprocess = \
            rlimit.rlimit_subprocess_create (argc, argv, envp)


    def run(self, timeout=None, memory=None):
        '''Non-blocking execution.

        The subprocess will be run in a separate thread. This function
        do not return anything but might throw an exception if a
        problem occurs at start time. The user might set a limit over
        the maximum time and memory for the subprocess to run.
        '''
        if not (timeout == None):
            rlimit.rlimit_set_time_limit(self.subprocess, timeout)

        if not (memory == None):
            rlimit.rlimit_set_memory_limit(self.subprocess, memory)

        rlimit.rlimit_subprocess_run(self.subprocess)


    def kill(self):
        '''Kill the process.'''
        if (rlimit.rlimit_subprocess_kill(self.subprocess) == -1):
            raise Exception("subprocess kill failed")


    def suspend(self):
        '''Suspend the process.'''
        if (rlimit.rlimit_subprocess_suspend(self.subprocess) == -1):
            raise Exception("subprocess suspend failed")


    def resume(self):
        '''Resume the process.'''
        if (rlimit.rlimit_subprocess_resume(self.subprocess) == -1):
            raise Exception("subprocess resume failed")


    def wait(self):
        '''Wait for the end of the execution.

        This command wait for the subprocess to end and returns with
        the subprocess return code.
        '''
        return rlimit.rlimit_subprocess_wait(self.subprocess)


    def write(self, msg):
        '''Write to the stdin of the subprocess.

        Write 'msg' to the stdin of the subprocess. Note that you need
        to be sure that the subprocess wait for input.
        '''
        rlimit_write_stdin(self.subprocess, c_char_p(msg))


    def expect(self, pattern, stdout=True, stderr=False, timeout=None):
        '''Search the given pattern at the end of the given output
        (default: stdout) and wait until the timeout elapsed if the
        pattern is not found yet.

        This command is intended to ease the interactive communication
        with the subprocess. It returns 'True' is the pattern has been
        found and 'False' otherwise.

        Example:

        process.run()
        process.expect('password:')
        processs.write('mypassword')
        if (process.expect('Welcome !\n%prompt> ')):
            print('Success !')
        elif (process.expect('Wrong password, try again!\npassword:'))
            process.expect('myotherpassword')
        else
            print('Cannot log-in !')
        '''
        if (timeout == None):
            timeout = 120

        if (stdout and stderr):
            return rlimit_expect(self.subprocess, pattern, timeout)
        elif (stdout and not stderr):
            return rlimit_expect_stdout(self.subprocess, pattern, timeout)
        elif (not stdout and stderr):
            return rlimit_expect_stderr(self.subprocess, pattern, timeout)

    def status(self):
        if (self.subprocess.contents.status == 0):
            return "Ready"
        elif (self.subprocess.contents.status == 1):
            return "Running"
        elif (self.subprocess.contents.status == 2):
            return "Sleeping"
        elif (self.subprocess.contents.status == 3):
            return "Stopped"
        elif (self.subprocess.contents.status == 4):
            return "Zombie"
        elif (self.subprocess.contents.status == 5):
            return "Terminated"
        elif (self.subprocess.contents.status == 6):
            return "Killed"
        elif (self.subprocess.contents.status == 7):
            return "Timeout"
        elif (self.subprocess.contents.status == 8):
            return "Memoryout"
        elif (self.subprocess.contents.status == 9):
            return "FsizeExceed"
        elif (self.subprocess.contents.status == 10):
            return "FDExceed"
        elif (self.subprocess.contents.status == 11):
            return "ProcExceed"
        elif (self.subprocess.contents.status == 12):
            return "DeniedSyscall"

    def stdout(self):
        return self.subprocess.contents.stdout_buffer


    def stderr(self):
        return self.subprocess.contents.stderr_buffer


    def returnvalue(self):
        return self.subprocess.contents.retval


    def profile(self):
        return self.subprocess.contents.status

# Testing the package
if __name__ == "__main__":
    subproc =  Subprocess(["/bin/ls", "-al"], None)

    print("Start")
    print("Status is '%s'" % subproc.status())
    print("Stdout = '%s'" % subproc.stdout())
    print("Stderr = '%s'" % subproc.stderr())
    print("Returnvalue = %i" % subproc.returnvalue())

    subproc.run()
    subproc.wait()

    print("")
    print("After wait:")
    print("Status is '%s'" % subproc.status())

    print("Stdout = '%s'" % subproc.stdout())
    print("Stdout size = %i" % libc.strlen(subproc.stdout()))

    print("Stderr = '%s'" % subproc.stderr())
    print("Stderr size = %i" % libc.strlen(subproc.stderr()))

    print("Returnvalue = %i" % subproc.returnvalue())
