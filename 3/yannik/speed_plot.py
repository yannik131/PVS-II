import matplotlib.pyplot as plt

# Elapsed times from threads 1 to 16
elapsed_times = [
    8.181025, 7.441424, 7.317599, 7.715001,
    7.576283, 7.952868, 7.743789, 8.130302,
    7.950556, 8.158019, 7.999256, 8.275160,
    8.122064, 8.158630, 8.413552, 8.289222
]

threads = list(range(1, 17))
T1 = elapsed_times[0]
speedup = [T1 / t for t in elapsed_times]

plt.plot(threads, speedup, marker='o')
plt.xlabel("Number of Threads")
plt.ylabel("Speedup")
plt.title("Speedup vs. Thread Count")
plt.grid(True)
plt.show()