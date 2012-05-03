'''
Subprocess is Python module for running, controlling and profiling
subprocesses in an automatic manner. It is primarely designed for
black-box testing purpose in an educational context.
'''

try:
    import os
    import resource
    import select
    import subprocess
    import threading
    import time
except ImportError, err:
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
                    soft, hard = resource.getrlimit(resource.RLIMIT_STACK)
                    if ((hard == -1) or (hard > memory)):
                        resource.setrlimit(resource.RLIMIT_STACK,
                                           (memory, hard))
                    # Setting limit on heap memory
                    soft, hard = resource.getrlimit(resource.RLIMIT_DATA)
                    if ((hard == -1) or (hard > memory)):
                        resource.setrlimit(resource.RLIMIT_DATA,
                                           (memory, hard))
            def target():
                '''Thread running the subprocess and collecting results.'''
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

                # Restore the limits afterwards
                resource.setrlimit(resource.RLIMIT_STACK, (-1, -1))
                resource.setrlimit(resource.RLIMIT_DATA, (-1, -1))

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



# Examples... to be removed later

print('Running...')
__process__ = Subprocess(['sleep', '5'])

__process__.run(memory=50000)
__process__.wait()
print("Return code: %d" % __process__.returncode)
print("Execution time: %f" % __process__.real_time)
print("User time: %f" % __process__.user_time)
print("System time: %f" % __process__.sys_time)
print("Memory: %f" % __process__.memory)


__process__.run(timeout=3)
__process__.wait()
print("Return code: %d" % __process__.returncode)
print("Execution time: %f" % __process__.real_time)

__process__.run(timeout=1)
__process__.wait()
print("Return code: %d" % __process__.returncode)
print("Execution time: %f" % __process__.real_time)
