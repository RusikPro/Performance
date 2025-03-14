#include "timer.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

/*----------------------------------------------------------------------------*/

template < typename T >
inline void doNotOptimize ( T const & value )
{
    asm volatile( "" : : "r,m"(value) : "memory" );
}

/*----------------------------------------------------------------------------*/

using Matrix = std::vector< std::vector< int > >;

Matrix generateMatrix ( int rows, int cols )
{
    Matrix matrix( rows, std::vector<int>(cols, 0) );
    for ( int i = 0; i < rows; ++i )
    {
        for ( int j = 0; j < cols; ++j )
        {
            matrix[i][j] = (i + j) % 256;
        }
    }
    return matrix;
}


long long
countWithContainer ( const Matrix& matrix, int threshold, int numThreads )
{
    int numRows = static_cast<int>(matrix.size());

    std::vector<long long> results(numThreads, 0);

    std::vector<std::thread> threads;
    threads.reserve( numThreads );

    // Determine chunk size (round up)
    int chunkSize = (numRows + numThreads - 1) / numThreads;

    for (int t = 0; t < numThreads; ++t)
    {
        int startRow = t * chunkSize;
        int endRow = std::min(startRow + chunkSize, numRows);
        threads.emplace_back([&, startRow, endRow, t]()
        {
            long long localCount = 0;
            for (int i = startRow; i < endRow; ++i) {
                for (int value : matrix[i]) {
                    if (value > threshold)
                        ++results[ t ];
                }
            }
            // Write the local count into the container
            results[ t ] = localCount;
        });
    }

    // Wait for all threads to complete
    for (auto& th : threads)
    {
        th.join();
    }

    // Combine partial results
    long long totalCount = 0;
    for (auto count : results)
    {
        totalCount += count;
    }
    return totalCount;
}

long long
countWithLocalCounter ( const Matrix& matrix, int threshold, int numThreads )
{
    int numRows = static_cast< int >( matrix.size() );

    std::atomic<long long> globalCount(0);
    std::vector<std::thread> threads;
    threads.reserve( numThreads );

    // Determine chunk size (round up)
    int chunkSize = ( numRows + numThreads - 1 ) / numThreads;

    for (int t = 0; t < numThreads; ++t )
    {
        int startRow = t * chunkSize;
        int endRow = std::min(startRow + chunkSize, numRows);

        threads.emplace_back( [&, startRow, endRow, t] ()
        {
            long long localCount = 0;
            for (int i = startRow; i < endRow; ++i)
            {
                for (int value : matrix[i] )
                {
                    if ( value > threshold )
                        ++localCount;
                }
            }
            // Atomically add the local counter to the global counter.
            globalCount.fetch_add( localCount, std::memory_order_relaxed );
        } );
    }

    for ( auto& th : threads )
    {
        th.join();
    }

    return globalCount.load();
}

/*----------------------------------------------------------------------------*/

// compute the average duration (in microseconds),
// and then write the results to a CSV file.
void runBenchmarks (
        const Matrix& image
    ,   int threshold
    ,   int maxThreads = 10
    ,   int iterations = 20
)
{
    std::vector<double> timesContainer( maxThreads, 0.0 );
    std::vector<double> timesLocal( maxThreads, 0.0 );

    // Volatile sinks to prevent compiler optimization.
    volatile long long sinkContainer = 0;
    volatile long long sinkLocal = 0;

    for (int numThreads = 1; numThreads <= maxThreads; ++numThreads)
    {
        double totalTimeContainer = 0.0;
        double totalTimeLocal = 0.0;
        for (int iter = 0; iter < iterations; ++iter)
        {
            {
                // Matrix image = generateMatrix(1000, 1000);
                Timer<std::micro> timer("countWithContainer");
                auto res = countWithContainer(image, threshold, numThreads);
                // sinkContainer += res;
                doNotOptimize(res);
                totalTimeContainer += timer.stop();
            }
            {
                // Matrix image = generateMatrix(1000, 1000);
                Timer<std::micro> timer("countWithLocalCounter");
                auto res = countWithLocalCounter(image, threshold, numThreads);
                // sinkLocal += res;
                doNotOptimize(res);
                totalTimeLocal += timer.stop();
            }
        }
        // Compute average times
        timesContainer[ numThreads - 1 ] = totalTimeContainer / iterations;
        timesLocal[ numThreads - 1 ] = totalTimeLocal / iterations;
    }

    // Construct output filename based on processor architecture.
    std::string arch = "MacOsM1";
    std::string filename = "benchmarks_" + arch + ".csv";
    std::ofstream ofs(filename);
    if (!ofs) {
        std::cerr << "Error: cannot open file " << filename << " for writing.\n";
        return;
    }

    // Write CSV file:
    // First row: header with thread counts
    ofs << "ThreadCount";
    for (int t = 1; t <= maxThreads; ++t) {
        ofs << "," << t;
    }
    ofs << "\n";

    // Second row: times for countWithContainer.
    ofs << "Container";
    for (int t = 0; t < maxThreads; ++t) {
        ofs << "," << timesContainer[t];
    }
    ofs << "\n";

    // Third row: times for countWithLocalCounter.
    ofs << "LocalCounter";
    for (int t = 0; t < maxThreads; ++t) {
        ofs << "," << timesLocal[t];
    }
    ofs << "\n";

    ofs.close();
    std::cout << "Benchmark results written to " << filename << std::endl;
}

int main ( int argc, char* argv[] )
{
    const int defaultRows = 1000;
    const int defaultCols = 1000;
    const int defaultMaxThreads = 30;
    const int threshold = 128;  // Hardcoded threshold.
    const int iterations = 20;  // Hardcoded number of iterations per thread count.

    // Check if benchmark mode is requested.
    if (argc > 1 && std::string(argv[1]) == "benchmark" )
    {
        // Use default values.
        int maxThreads = defaultMaxThreads;
        int rows = defaultRows;
        int cols = defaultCols;

        // Parse named parameters.
        for (int i = 2; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (arg == "--threads" && (i + 1) < argc)
            {
                maxThreads = std::stoi(argv[++i]);
            }
            else if (arg == "--rows" && (i + 1) < argc)
            {
                rows = std::stoi(argv[++i]);
            }
            else if ( arg == "--cols" && (i + 1) < argc )
            {
                cols = std::stoi(argv[++i]);
            }
        }

        std::cout
            << "Running benchmarks with rows=" << rows
            << ", cols=" << cols
            << ", maxThreads=" << maxThreads
            << ", iterations=" << iterations << std::endl
        ;

        // Generate a deterministic matrix.
        Matrix image = generateMatrix(rows, cols);

        // Run the benchmarks.
        runBenchmarks(image, threshold, maxThreads, iterations);
    }
    else
    {
        const int numThreads = 10;

        Matrix image = generateMatrix( defaultRows, defaultCols );

        {
            Timer< std::micro > timer( "countWithContainer" );
            long long count1 = countWithContainer(image, threshold, numThreads);
        }

        {
            Timer< std::micro > timer( "countWithLocalCounter" );
            long long count1 = countWithLocalCounter(image, threshold, numThreads);
        }
    }
    return 0;
}
