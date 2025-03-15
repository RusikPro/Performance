#!/usr/bin/env python3
import sys
import os
import pandas as pd
import matplotlib.pyplot as plt
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

def read_and_plot(filename, ax):
    """
    Read the CSV file and update the plot on the given axes.
    Assumes the file has the following structure:
      Row 0: "ThreadCount", t1, t2, ..., tN
      Row 1: "ContainerStd", std1, std2, ..., stdN
      Row 3: "LocalCounterStd", std1, std2, ..., stdN
    """
    try:
        df = pd.read_csv(filename, header=None)
    except Exception as e:
        print(f"Error reading file: {e}")
        return

    try:
        thread_counts = list(map(int, df.iloc[0, 1:].tolist()))
        container_std = list(map(float, df.iloc[1, 1:].tolist()))
        localcounter_std = list(map(float, df.iloc[3, 1:].tolist()))
    except Exception as e:
        print(f"Error processing CSV data: {e}")
        return

    ax.clear()
    ax.plot(thread_counts, container_std, marker='o', label='ContainerStd')
    ax.plot(thread_counts, localcounter_std, marker='s', label='LocalCounterStd')
    ax.set_xlabel('Number of Threads')
    ax.set_ylabel('Standard Deviation (μs)')
    ax.set_title('Benchmark Spread (Standard Deviation)')
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
            import time
            now = time.time()
            # Debounce: only update if at least 1 second has passed
            if now - self.last_update < 1:
                return
            self.last_update = now
            print("File changed; updating plot...")
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
    # Use a blocking show() call so that the window only updates when the file is modified.
    plt.show(block=True)

    observer.stop()
    observer.join()
    print("Exiting...")

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python watch_plot_std.py <benchmark_file.csv>")
        sys.exit(1)
    main(sys.argv[1])
