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
except OSError as err:
    raise OSError (str(err) + '''

librlimit.so cannot be found on your system.
You should install it and/or set up properly your system.
''')

class SUBPROCESS(Structure):
    _fields_ = [("status", c_int),
                ("retval", c_int),
                ("stdout_buffer", c_char_p),
                ("stderr_buffer", c_char_p)]

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
        subprocess = POINTER(SUBPROCESS)
        subprocess_pointer = \
            rlimit.rlimit_subprocess_create (argc, argv, envp)

        self.subprocess = subprocess_pointer.contents

        # Subprocess information
        self.status = self.subprocess.status
        self.returncode = c_int(self.subprocess.retval)
        self.stdout = c_char_p(self.subprocess.stdout_buffer)
        self.stderr = c_char_p(self.subprocess.stderr_buffer)

        # Subprocess profile information
        self.real_time = None
        self.user_time = None
        self.sys_time = None
        self.memory = None

    def run(self, timeout=None, memory=None):
        '''Non-blocking execution.

        The subprocess will be run in a separate thread. This function
        do not return anything but might throw an exception if a
        problem occurs at start time. The user might set a limit over
        the maximum time and memory for the subprocess to run.
        '''
        pass

    def kill(self):
        '''Kill the process.'''
        pass

    def suspend(self):
        '''Suspend the process.'''
        pass

    def resume(self):
        '''Resume the process.'''
        pass
    
    def wait(self):
        '''Wait for the end of the execution.

        This command wait for the subprocess to end and returns with
        the subprocess return code.
        '''
        pass
    
    def write(self, msg):
        '''Write to the stdin of the subprocess.

        Write 'msg' to the stdin of the subprocess. Note that you need
        to be sure that the subprocess wait for input.
        '''
        pass

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
        pass

if __name__ == "__main__":
        subproc =  Subprocess(["/bin/ls", "-a"], None)
