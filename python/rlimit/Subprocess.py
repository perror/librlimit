'''
Subprocess is a Python class to control and profile processes in a
convenient and flexible manner. It is primarily designed for black-box
testing over untrusted code in an educational context. It is also
fully based on the C library librlimit.
'''

try:
    import ctypes
except ImportError as err:
    raise ImportError (str(err) + '''

A needed module for Subprocess was not found.
You should install it and try again.
''')

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

        # Subprocess information
        self.status = None
        self.stdout = None
        self.stderr = None
        self.returncode = None

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
