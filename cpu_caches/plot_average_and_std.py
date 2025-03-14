#!/usr/bin/env python3
import sys
import os
import time
import pandas as pd
import matplotlib.pyplot as plt
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

def read_and_plot(filename, axes):
    """
    Read the CSV file and update the two subplots.

    Expected CSV structure:
      Row 0: "ThreadCount", t1, t2, ..., tN
      Row 1: "ContainerAvg", avg1, avg2, ..., avgN
      Row 2: "ContainerStd", std1, std2, ..., stdN
      Row 3: "LocalCounterAvg", avg1, avg2, ..., avgN
      Row 4: "LocalCounterStd", std1, std2, ..., stdN
    """
    try:
        df = pd.read_csv(filename, header=None)
    except Exception as e:
        print(f"Error reading file: {e}")
        return

    try:
        thread_counts = list(map(int, df.iloc[0, 1:].tolist()))
        container_avg   = list(map(float, df.iloc[1, 1:].tolist()))
        container_std   = list(map(float, df.iloc[2, 1:].tolist()))
        local_avg       = list(map(float, df.iloc[3, 1:].tolist()))
        local_std       = list(map(float, df.iloc[4, 1:].tolist()))
    except Exception as e:
        print(f"Error processing CSV data: {e}")
        return

    ax1, ax2 = axes

    # Clear the axes and replot
    ax1.clear()
    ax1.plot(thread_counts, container_avg, marker='o', label='ContainerAvg')
    ax1.plot(thread_counts, local_avg, marker='s', label='LocalCounterAvg')
    ax1.set_ylabel('Average Time (μs)')
    ax1.set_title('Benchmark Averages')
    ax1.legend()
    ax1.grid(True)

    ax2.clear()
    ax2.plot(thread_counts, container_std, marker='o', label='ContainerStd')
    ax2.plot(thread_counts, local_std, marker='s', label='LocalCounterStd')
    ax2.set_xlabel('Number of Threads')
    ax2.set_ylabel('Std Dev (μs)')
    ax2.set_title('Benchmark Spread (Standard Deviation)')
    ax2.legend()
    ax2.grid(True)

    plt.tight_layout()
    plt.draw()

class CSVChangeHandler(FileSystemEventHandler):
    def __init__(self, filename, axes):
        self.filename = os.path.abspath(filename)
        self.axes = axes
        self.last_update = 0  # Timestamp of last update

    def on_modified(self, event):
        # Update only if the modified file is exactly our target CSV.
        if os.path.abspath(event.src_path) == self.filename:
            now = time.time()
            # Debounce: update only if at least 1 second has passed.
            if now - self.last_update < 1:
                return
            self.last_update = now
            print("File changed; updating plot...")
            read_and_plot(self.filename, self.axes)

def main(filename):
    # Enable interactive mode and create a figure with two subplots.
    plt.ion()
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 12), sharex=True)
    read_and_plot(filename, (ax1, ax2))

    # Attempt to disable "always on top" if supported (for TkAgg backend).
    try:
        manager = plt.get_current_fig_manager()
        manager.window.attributes('-topmost', False)
    except Exception as e:
        print("Could not disable always-on-top:", e)

    # Set up the watchdog observer.
    event_handler = CSVChangeHandler(filename, (ax1, ax2))
    observer = Observer()
    directory = os.path.dirname(os.path.abspath(filename))
    observer.schedule(event_handler, path=directory, recursive=False)
    observer.start()

    print("Monitoring file for changes. Close the plot window to exit.")
    # Use a blocking plt.show() to run the GUI event loop.
    plt.show(block=True)

    observer.stop()
    observer.join()
    print("Exiting...")

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python plot_average_and_std.py <benchmark_file.csv>")
        sys.exit(1)
    main(sys.argv[1])
