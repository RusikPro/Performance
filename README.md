# Performance

Performance is a collection of benchmarks and performance measurement tools designed to evaluate CPU cache behavior and multi-threading performance. The repository currently includes a module for benchmarking two methods of counting matrix elements in C++ and a Python plotting tool to visualize the benchmark results. Additional modules will be added in the future.

## Module: CPU Caches

The `cpu_caches` module contains benchmarks and research exploring CPU cache behavior. Currently, it includes:

- **`parallel_chunks.cpp`**  
  Demonstrates functionality for multi-threaded counting of matrix elements above a given threshold. This code compares two approaches:  
  - **`countWithContainer`** – Uses a vector container to store partial counts from each thread.  
  - **`countWithLocalCounter`** – Uses local counters aggregated into an atomic global counter.

  The benchmark generates matrices (deterministic or random), executes the counting with various thread counts, measures execution times, and writes the results to a CSV file (e.g., `benchmarks_<arch>.csv`).

- **`plot_parallel_chunks_result.py`**  
  A Python script that reads the CSV file generated by `parallel_chunks.cpp` and plots the benchmark results. It shows average execution times, standard deviations, and automatically updates when the CSV file changes.

More cache-specific examples and experiments will be added in the future to broaden the research in this module.


## Getting Started

### Prerequisites

- **C++ Code:**
  - A C++ compiler with C++11 (or later) support.
  - A build system (e.g., CMake or Make) if desired.
  - POSIX threads support (typically included with your compiler).

- **Python Script:**
  - Python 3.x
  - The following Python libraries:
    - `pandas`
    - `matplotlib`

### Building and Running the C++ Benchmark

1. **Compile the Code**

   For example, using `g++`:

   ```bash
   g++ -std=c++20 -O3 -pthread -o parallel_chunks parallel_chunks.cpp
   ```

2. **Run the Benchmark**

   To run the benchmarks, execute the compiled binary with the `benchmark` argument. You can also specify optional parameters:

   ```bash
   ./parallel_chunks benchmark [options]
   ```

   **Options:**
   - `--threads <number>`: Maximum number of threads to test (default: 30)
   - `--rows <number>`: Number of rows in the matrix (default: 1000)
   - `--cols <number>`: Number of columns in the matrix (default: 1000)
   - `--rowscols <number>`: Number of rows and columns in the matrix (default: 1000)
   - `--iterations <number>`: Number of iterations per thread count (default: 5)

   **Example:**

   ```bash
   ./parallel_chunks benchmark --threads 10 --rows 500 --cols 500 --iterations 10
   ```

   The benchmark results will be saved in a CSV file (e.g., `benchmarks_MacOsM1.csv`).

### Running the Python Plotting Tool

1. **Ensure Dependencies are Installed**

   You can install the required libraries using `pip`:

   ```bash
   pip install pandas matplotlib
   ```

2. **Execute the Plotting Script**

   Run the script and provide the path to the CSV file generated by the benchmark:

   ```bash
   python3 plot_parallel_chunks_results.py <path_to_csv_file> [--vertical]
   ```

   **Example:**

   ```bash
   python3 plot_parallel_chunks_results.py benchmarks_MacOsM1.csv
   ```

   Use the `--vertical` flag to arrange the plots vertically instead of side-by-side.

## Repository Structure

```
Performance/
├── cpu_caches/
│   ├── parallel_chunks.cpp              # C++ benchmark code
│   ├── timer.h                          # Timer utility header
│   |── plot_parallel_chunks_results.py  # Python plotting script for averages and standard deviations
|   |── plot_averages.py                 # Python plotting script only for averages
|   └── plot_stds.py                     # Python plotting script only for standard deviations
└── README.md                            # This file
```

## Future Work

New modules and benchmarks will be added to extend the performance analysis across various aspects of system architecture and multi-threading. Contributions, suggestions, and feedback are welcome!

## Contributing

Contributions to the project are encouraged. To contribute:
1. Fork the repository.
2. Create a feature branch.
3. Commit your changes with clear messages.
4. Open a pull request detailing your modifications.

Please adhere to the coding style.

## License

This project is licensed under the [MIT License](LICENSE).

## Contact

For any questions or suggestions, please open an issue on GitHub or contact the maintainers.
