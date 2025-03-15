#include "timer.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
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
    return Matrix( rows, std::vector<int>(cols, 150) );
}

std::unique_ptr< Matrix > generateMatrixOnHeap ( int rows, int cols )
{
    auto pMatrix = std::make_unique<Matrix>(rows, std::vector<int>(cols, 0));
    for ( int i = 0; i < rows; ++i )
    {
        for ( int j = 0; j < cols; ++j )
        {
            // matrix[i][j] = ((i + j) * 100) % 256;
            (*pMatrix)[i][j] = ( i % 2 == 1 && j % 2 == 1 ) ? 150 : 100;
        }
    }
    return pMatrix;
}

Matrix generateRandomMatrix ( int rows, int cols )
{
    Matrix matrix(rows, std::vector<int>(cols, 0));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 255);

    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            matrix[i][j] = dist(gen);
        }
    }
    return matrix;
}

/*----------------------------------------------------------------------------*/

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

template <typename Func>
std::vector<std::vector<double>> runBenchmark (
    int maxThreads, int iterations, int threshold, int rows, int cols,
    Func benchmarkFunc, const std::string & timerLabel
)
{
    std::vector<std::vector<double>> containerTimes(maxThreads);
    for (int numThreads = 1; numThreads <= maxThreads; ++numThreads)
    {
        for (int iter = 0; iter < iterations; ++iter)
        {
            auto pImage = generateMatrix(rows, cols);
            Timer<std::micro> timer(timerLabel);
            auto res = benchmarkFunc(pImage, threshold, numThreads);
            doNotOptimize(res);
            containerTimes[numThreads - 1].push_back(timer.stop());
        }
    }
    return containerTimes;
}

/*----------------------------------------------------------------------------*/

double computeStdDev ( std::vector< double > const & values, double avg )
{
    double sumSq = 0.0;
    for ( double v : values )
    {
        double diff = v - avg;
        sumSq += diff * diff;
    }
    return std::sqrt(sumSq / values.size());
}

void computeStatistics (
    const std::vector<std::vector<double>>& times,
    std::vector<double>& avg,
    std::vector<double>& stdDev,
    int iterations)
{
    int maxThreads = static_cast<int>(times.size());
    avg.resize(maxThreads, 0.0);
    stdDev.resize(maxThreads, 0.0);

    for (int i = 0; i < maxThreads; ++i)
    {
        double sum = 0.0;
        for (double t : times[i])
            sum += t;
        avg[i] = sum / iterations;
        stdDev[i] = computeStdDev(times[i], avg[i]);
    }
}

/*----------------------------------------------------------------------------*/

void writeResultsToCSV (
    const std::string& arch,
    const std::vector<double>& containerAvg,
    const std::vector<double>& containerStd,
    const std::vector<double>& localAvg,
    const std::vector<double>& localStd)
{
    std::string filename = "benchmarks_" + arch + ".csv";
    std::ofstream ofs(filename);
    if ( !ofs )
    {
        std::cerr
            << "Error: cannot open file " << filename << " for writing.\n"
        ;
        return;
    }

    int maxThreads = static_cast<int>(containerAvg.size());

    // Write CSV header
    ofs << "ThreadCount";
    for (int t = 1; t <= maxThreads; ++t)
        ofs << "," << t;
    ofs << "\n";

    // Write rows for each metric.
    ofs << "ContainerAvg";
    for (double v : containerAvg)
        ofs << "," << v;
    ofs << "\n";

    ofs << "ContainerStd";
    for (double v : containerStd)
        ofs << "," << v;
    ofs << "\n";

    ofs << "LocalCounterAvg";
    for (double v : localAvg)
        ofs << "," << v;
    ofs << "\n";

    ofs << "LocalCounterStd";
    for (double v : localStd)
        ofs << "," << v;
    ofs << "\n";

    ofs.close();
    std::cout << "Benchmark results written to " << filename << std::endl;
}

/*----------------------------------------------------------------------------*/

// compute the average duration (in microseconds),
// and then write the results to a CSV file.
void runBenchmarks (
        const Matrix& image
    ,   int threshold
    ,   int maxThreads = 10
    ,   int iterations = 20
    ,   int rows = 1000
    ,   int cols = 1000
)
{
    // Run benchmarks.
    auto containerTimes = runBenchmark(
        maxThreads, iterations, threshold, rows, cols,
        countWithContainer, "countWithContainer"
    );

    auto localTimes = runBenchmark(
        maxThreads, iterations, threshold, rows, cols,
        countWithLocalCounter, "countWithLocalCounter"
    );

    // Compute statistics.
    std::vector<double> containerAvg, containerStd;
    std::vector<double> localAvg, localStd;
    computeStatistics(containerTimes, containerAvg, containerStd, iterations);
    computeStatistics(localTimes, localAvg, localStd, iterations);

    // Write results.
    std::string arch = "MacOsM1"; // TODO: determine automaticlly.
    writeResultsToCSV(arch, containerAvg, containerStd, localAvg, localStd);
}

/*----------------------------------------------------------------------------*/

int main ( int argc, char* argv[] )
{
    const int defaultRows = 1000;
    const int defaultCols = 1000;
    const int defaultMaxThreads = 30;
    const int threshold = 128;
    const int defaultIterations = 5;

    // Check if benchmark mode is requested.
    if (argc > 1 && std::string(argv[1]) == "benchmark" )
    {
        // Use default values.
        int maxThreads = defaultMaxThreads;
        int rows = defaultRows;
        int cols = defaultCols;
        int iterations = defaultIterations;

        // Parse named parameters.
        for (int i = 2; i < argc; ++i)
        {
            std::string arg = argv[i];
            if ( arg == "--threads" && (i + 1) < argc )
            {
                maxThreads = std::stoi(argv[++i]);
            }
            else if ( arg == "--rows" && (i + 1) < argc )
            {
                rows = std::stoi(argv[++i]);
            }
            else if ( arg == "--cols" && (i + 1) < argc )
            {
                cols = std::stoi(argv[++i]);
            }
            else if ( arg == "--rowscols" && (i + 1) < argc )
            {
                rows = std::stoi(argv[++i]);
                cols = std::stoi(argv[i]);
            }
            else if ( arg == "--iterations" && (i + 1) < argc )
            {
                iterations = std::stoi(argv[++i]);
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
        runBenchmarks( image, threshold, maxThreads, iterations, rows, cols );
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
