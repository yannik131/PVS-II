import subprocess
import re
import matplotlib.pyplot as plt

times = []
threads = range(1, 17)

for t in threads:
    result = subprocess.run(
        [r".\3_3.exe", str(t)],
        capture_output=True, text=True, shell=True
    )
    print(result.stdout)
    match = re.search(r"Elapsed time:\s*([\d.]+)s", result.stdout)
    if match:
        times.append(float(match.group(1)))
    else:
        times.append(None)

plt.plot(threads, times, marker='o')
plt.xlabel("Number of Threads")
plt.ylabel("Elapsed Time (s)")
plt.title("Execution Time vs. Thread Count")
plt.grid(True)
plt.show()
