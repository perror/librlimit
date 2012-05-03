from rlimit import rlimit

print('Running...')
print("")
__process__ = rlimit.Subprocess(['sleep', '3'])

print("First run:")
__process__.run(memory=50000)
__process__.wait()
print(("Return code: %d" % __process__.returncode))
print(("Execution time: %f" % __process__.real_time))
print(("User time: %f" % __process__.user_time))
print(("System time: %f" % __process__.sys_time))
print(("Memory: %f" % __process__.memory))
print("")

print("Second run:")
__process__.run(timeout=5)
__process__.wait()
print(("Return code: %d" % __process__.returncode))
print(("Execution time: %f" % __process__.real_time))
print("")

print("Third run:")
__process__.run(timeout=1)
__process__.wait()
print(("Return code: %d" % __process__.returncode))
print(("Execution time: %f" % __process__.real_time))
