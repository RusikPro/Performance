#!/usr/bin/env python3
import sys
import os
import time
import pandas as pd
import matplotlib.pyplot as plt
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

def read_and_plot(filename, ax):
    """
    Read the CSV file and update the average plot on the given axes.
    Assumes the file has the following structure:
      Row 0: "ThreadCount", t1, t2, ..., tN
      Row 1: "ContainerAvg", avg1, avg2, ..., avgN
      Row 3: "LocalCounterAvg", avg1, avg2, ..., avgN
    """
    try:
        df = pd.read_csv(filename, header=None)
    except Exception as e:
        print(f"Error reading file: {e}")
        return

    try:
        thread_counts = list(map(int, df.iloc[0, 1:].tolist()))
        container_avg = list(map(float, df.iloc[1, 1:].tolist()))
        localcounter_avg = list(map(float, df.iloc[3, 1:].tolist()))
    except Exception as e:
        print(f"Error processing CSV data: {e}")
        return

    ax.clear()
    ax.plot(thread_counts, container_avg, marker='o', label='ContainerAvg')
    ax.plot(thread_counts, localcounter_avg, marker='s', label='LocalCounterAvg')
    ax.set_xlabel('Number of Threads')
    ax.set_ylabel('Average Time (μs)')
    ax.set_title('Benchmark Averages')
    ax.legend()
    ax.grid(True)
    plt.draw()

class CSVChangeHandler(FileSystemEventHandler):
    def __init__(self, filename, ax):
        self.filename = os.path.abspath(filename)
        self.ax = ax
        self.last_update = 0  # Timestamp of last update

    def on_modified(self, event):
        if os.path.abspath(event.src_path) == self.filename:
            now = time.time()
            # Debounce: update only if at least 1 second has passed
            if now - self.last_update < 1:
                return
            self.last_update = now
            print("File changed; updating average plot...")
            read_and_plot(self.filename, self.ax)

def main(filename):
    # Enable interactive mode and create the plot.
    plt.ion()
    fig, ax = plt.subplots(figsize=(10, 6))
    read_and_plot(filename, ax)

    # Attempt to disable always-on-top (for TkAgg backend).
    try:
        manager = plt.get_current_fig_manager()
        manager.window.attributes('-topmost', False)
    except Exception as e:
        print("Could not disable always-on-top:", e)

    # Set up the watchdog observer.
    event_handler = CSVChangeHandler(filename, ax)
    observer = Observer()
    directory = os.path.dirname(os.path.abspath(filename))
    observer.schedule(event_handler, path=directory, recursive=False)
    observer.start()

    print("Monitoring file for changes. Close the plot window to exit.")
    plt.show(block=True)

    observer.stop()
    observer.join()
    print("Exiting...")

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python watch_plot_avg.py <benchmark_file.csv>")
        sys.exit(1)
    main(sys.argv[1])
