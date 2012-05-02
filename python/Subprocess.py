'''
Subprocess is Python module for running, controlling and profiling
subprocesses in an automatic manner. It is primarely designed for
black-box testing purpose in an educational context.
'''

try:
    import resource
    import select
    import subprocess
    import threading
    import time
except ImportError, err:
    raise ImportError (str(err) + '''

A module required by the Subprocess module was not found.
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
        self.process = None

        # Subprocess output
        self.stdout = None
        self.stderr = None
        self.returncode = None

        # Subprocess profile information
        self.real_time = None
        self.user_time = None
        self.sys_time = None
        self.memory = None

        self.usage = resource.getrusage(resource.RUSAGE_CHILDREN)

    def run(self, timeout=None, memoryout=None):
        '''Non-blocking execution.

        The subprocess will be run in a separate thread. This function
        do not return anything but might throw an exception if a
        problem occurs at start time. The user might set a limit over
        the maximum time and memory for the subprocess to run.
        '''
        def target():
            '''Inner target() function.'''
            self.process = subprocess.Popen(self.cmd,
                                            stdin =subprocess.PIPE,
                                            stdout=subprocess.PIPE,
                                            stderr=subprocess.PIPE,
                                            env=self.env)
            self.stdout, self.stderr = self.process.communicate()

        if (memoryout is not None):
            _, hard = resource.getrlimit(resource.RLIMIT_AS)
            resource.setrlimit(resource.RLIMIT_AS, (memoryout, hard))

        thread = threading.Thread(target=target)
        thread.start()
        start_time = time.time()

        thread.join(timeout)
        if thread.is_alive():
            self.process.terminate()
            thread.join()

        self.real_time = time.time() - start_time
        self.user_time = self.usage.ru_utime
        self.sys_time =  self.usage.ru_stime
        self.memory = self.usage.ru_maxrss

        self.returncode = self.process.returncode
        return self.process.returncode

    def wait(self):
        '''Restore blocking execution.

        This command wait for the subprocess to end and returns with
        the subprocess return code.
        '''
        return self.process.wait()

    def read(self):
        '''Read (stdout, stderr) from the subprocess.'''
        return (self.stdout, self.stderr)

    def write(self, msg):
        '''Write to the stdin of the subprocess.

        Write 'msg' to the stdin of the subprocess. Note that you need
        to be sure that the subprocess wait for input.
        '''
        while (True):
            _, inputready, _ = \
                select.select([], [self.process.stdin], [])

            if inputready.count() == 1:
                self.process.stdin.write(msg)
                self.process.stdin.flush()
                break

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

    def poll(self):
        '''Get the current subprocess status.

        Returns the returncode attribute ('None' means the subprocess
        is still running).
        '''
        return self.returncode

    def terminate(self):
        '''Terminate the subprocess.

        Terminate the subprocess and returns its return code.
        '''
        return self.process.terminate()




print('Running...')
__command__ = Subprocess(['/bin/ls'])

__command__.run()
print("Return code: %d" % __command__.returncode)
print("Execution time: %f" % __command__.real_time)
print("User time: %f" % __command__.user_time)
print("System time: %f" % __command__.sys_time)
print("Memory: %f" % __command__.memory)


#__command__.run(timeout=3)
#print("Return code: %d" % __command__.returncode)
#print("Execution time: %f" % __command__.real_time)

#__command__.run(timeout=1)
#print("Return code: %d" % __command__.returncode)
#print("Execution time: %f" % __command__.real_time)
