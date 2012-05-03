'''
rlimit is Python module for running, controlling and profiling
subprocesses in an automatic manner. It is primarily designed for
black-box testing purpose in an educational context.
'''

try:
    import resource
    import select
    import subprocess
    import threading
    import time
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

    def run(self, timeout=None, memory=None):
        '''Non-blocking execution.

        The subprocess will be run in a separate thread. This function
        do not return anything but might throw an exception if a
        problem occurs at start time. The user might set a limit over
        the maximum time and memory for the subprocess to run.
        '''
        def monitor():
            '''Monitor thread to timeout the subprocess thread when needed.'''
            def setlimits(memory):
                if (memory is not None):
                    # Setting limit on stack memory
                    _, hard = resource.getrlimit(resource.RLIMIT_STACK)
                    if ((hard == -1) or (hard > memory)):
                        resource.setrlimit(resource.RLIMIT_STACK,
                                           (memory, hard))
                    # Setting limit on heap memory
                    _, hard = resource.getrlimit(resource.RLIMIT_DATA)
                    if ((hard == -1) or (hard > memory)):
                        resource.setrlimit(resource.RLIMIT_DATA,
                                           (memory, hard))
            def target():
                '''Thread running the subprocess and collecting results.'''
                # Save the limit values
                if (memory is not None):
                    stack_soft, stack_hard = \
                        resource.getrlimit(resource.RLIMIT_STACK)
                    data_soft, data_hard = \
                        resource.getrlimit(resource.RLIMIT_DATA)

                self.process = subprocess.Popen(self.cmd,
                                                stdin =subprocess.PIPE,
                                                stdout=subprocess.PIPE,
                                                stderr=subprocess.PIPE,
                                                preexec_fn=setlimits(memory),
                                                env=self.env)
                start = time.time()
                self.stdout, self.stderr = self.process.communicate()
                self.real_time = (time.time() - start)

                usage = resource.getrusage(resource.RUSAGE_CHILDREN)
                self.user_time = usage.ru_utime
                self.sys_time =  usage.ru_stime
                self.memory = usage.ru_maxrss

                self.returncode = self.process.returncode

                # Restore the limit values afterwards
                if (memory is not None):
                    resource.setrlimit(resource.RLIMIT_STACK,
                                       (stack_soft, stack_hard))
                    resource.setrlimit(resource.RLIMIT_DATA,
                                       (data_soft, data_hard))

            target_thread = threading.Thread(target=target)
            target_thread.start()

            target_thread.join(timeout)
            if target_thread.is_alive():
                self.process.terminate()
                target_thread.join()

        monitor_thread = threading.Thread(target=monitor)
        monitor_thread.start()
        # Delaying until the process is launched
        while (True):
            time.sleep(0.15)
            if (self.process is not None):
                break

    def wait(self):
        '''Restore blocking execution.

        This command wait for the subprocess to end and returns with
        the subprocess return code.
        '''
        while (True):
            time.sleep(0.15)
            if (self.process.poll() is not None):
                break
        return self.process.returncode

    def write(self, msg):
        '''Write to the stdin of the subprocess.

        Write 'msg' to the stdin of the subprocess. Note that you need
        to be sure that the subprocess wait for input.
        '''
        while (True):
            _, inputready, _ = \
                select.select([], [self.process.stdin], [])

            if inputready[0] == self.process.stdin:
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
        return self.process.returncode

    def terminate(self):
        '''Terminate the subprocess.

        Terminate the subprocess and returns its return code.
        '''
        return self.process.terminate()
