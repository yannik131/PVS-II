import subprocess
import re
import matplotlib.pyplot as plt

times = []
threads = range(1, 30)

for t in threads:
    result = subprocess.run(
        [r".\4_1.exe", str(t), "10"],
        capture_output=True, text=True, shell=True
    )
    print(result.stdout)
    match = re.search(r"Elapsed time:\s*([\d.]+)", result.stdout)
    if match:
        value = float(match.group(1))
        if "ms" in result.stdout:
            value *= 1000
        times.append(value)
    else:
        times.append(None)

plt.plot(threads, times, marker='o')
plt.xlabel("Number of Threads")
plt.ylabel("Elapsed Time (us)")
plt.title("Execution Time vs. Thread Count")
plt.grid(True)
plt.show()
