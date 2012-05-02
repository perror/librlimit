import subprocess, threading, time

class Subprocess(object):
    def __init__(self, cmd, timeout=None, memory=None):
        self.cmd = cmd
        self.process = None

        # Input/Output
        self.stdin = None
        self.stdout = None
        self.stderr = None
        self.returncode = None

        # Private fields
        self.start = None
        
        # Resource limitations
        self.limits.timeout = timeout
        self.limits.memory = memory

        # Profile information
        self.profile.real_time = None
        self.profile.user_time = None
        self.profile.sys_time = None
        self.profile.memory = None

    def run():
        '''Non-blocking execution.

        The subprocess will be run in a separate thread. This function
        do not return anything but might throw an exception if a
        problem occurs at start time.
        '''
        def target():
            self.process = subprocess.Popen(self.cmd,
                                            stdin=subprocess.PIPE,
                                            stdout=subprocess.PIPE,
                                            stderr=subprocess.PIPE,
                                            shell=True)
            self.stdout, self.stderr = self.process.communicate()

        self.start = time.time()
        thread = threading.Thread(target=target)
        thread.start()

        thread.join(self.limit.timeout)
        if thread.is_alive():
            self.process.terminate()
            thread.join()

        self.returncode = self.process.returncode
        return self.process.returncode

    def wait():
        '''Restore blocking execution.

        This command wait for the subprocess to end and returns with
        the subprocess return code.
        '''
        pass

    def write(msg):
        '''Write to the stdin of the subprocess.

        Write 'msg' to the stdin of the subprocess. Note that you need
        to be sure that the subprocess wait for input.
        '''
        pass

    def poll():
        '''Get the current subprocess status.

        Returns current subprocess status (NotRunning, Running, Finished).
        '''
        pass
    
    def kill():
        '''Kill the subprocess.

        Kill the subprocess and returns its return code.
        '''
        pass


print('Hello!')
