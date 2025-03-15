#!/usr/bin/env python3
import argparse
import os
import sys
import time
import threading
import pandas as pd
import matplotlib
import matplotlib.pyplot as plt

def update_plot(ax1, ax2, file_path):
    """Clear ax1, ax2 and re-plot data from 'file_path'."""
    ax1.clear()
    ax2.clear()

    df = pd.read_csv(file_path, header=None)
    x = df.iloc[0, 1:].astype(float)

    container_avg = df.iloc[1, 1:].astype(float)
    container_std = df.iloc[2, 1:].astype(float)
    local_avg     = df.iloc[3, 1:].astype(float)
    local_std     = df.iloc[4, 1:].astype(float)

    # Plot Averages
    ax1.plot(x, container_avg, marker='o', label='ContainerAvg')
    ax1.plot(x, local_avg,     marker='o', label='LocalCounterAvg')
    ax1.set_title('Averages')
    ax1.set_xlabel('ThreadCount')
    ax1.set_ylabel('Average')
    ax1.legend()

    # Plot Standard Deviations
    ax2.plot(x, container_std, marker='o', label='ContainerStd')
    ax2.plot(x, local_std,     marker='o', label='LocalCounterStd')
    ax2.set_title('Standard Deviations')
    ax2.set_xlabel('ThreadCount')
    ax2.set_ylabel('Std')
    ax2.legend()

    ax1.figure.tight_layout()
    ax1.figure.canvas.draw_idle()  # Redraw without blocking



def watch_file(file_path, ax1, ax2, check_interval=1.0):
    """
    Background thread that checks for CSV file changes every 'check_interval'
    seconds and updates the plot if changed.
    """
    last_mod_time = os.path.getmtime(file_path)
    while plt.fignum_exists(ax1.figure.number):
        time.sleep(check_interval)
        # If the figure was closed, exit the thread
        if not plt.fignum_exists(ax1.figure.number):
            break

        try:
            current_mod_time = os.path.getmtime(file_path)
            if current_mod_time != last_mod_time:
                last_mod_time = current_mod_time
                print("File changed, updating plot...")
                update_plot(ax1, ax2, file_path)
        except Exception as e:
            print("Error watching file:", e)

def main():
    parser = argparse.ArgumentParser(description='Plot benchmark CSV data.')
    parser.add_argument('file', help='Path to the CSV file')
    parser.add_argument('--vertical', action='store_true',
                        help='Arrange plots vertically instead of side-by-side')
    args = parser.parse_args()

    # Create a single figure/axes pair for the entire session
    if args.vertical:
        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))
    else:
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))

    # Initial plot
    update_plot(ax1, ax2, args.file)

    # Start the background watcher thread
    watcher = threading.Thread(
        target=watch_file,
        args=(args.file, ax1, ax2),
        daemon=True
    )
    watcher.start()

    print("Initial plot displayed. Close the window (or press Ctrl+C) to exit.")
    # Block here so the window is fully interactive
    plt.show()

    # Once the user closes the window, plt.show() returns immediately.
    print("Exiting...")

if __name__ == "__main__":
    main()
