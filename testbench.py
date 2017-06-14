# Initial author: Luke Horgan
# Modified by: Greg Cusano
import os
import datetime

threads = [16, 4, 1]
nums = [("18446744073709551616", "40"), ("4611686018427387904", "40"), ("100000000000", "1000")] #2^64, 2^62, 10^11
for start, count in nums:
    for thread in threads:
        durations = []
        for i in range(0, 3):
            print("Trial %i: %s %s %s" % (i, thread, start, count))
            start_time = datetime.datetime.now()
            os.system("./main %s %s %s > /dev/null 2>&1" % (thread, start, count))
            durations.append((datetime.datetime.now() - start_time).total_seconds())
        print("%s %s %s: %s seconds\n" % (thread, start, count, sorted(durations)[1]))