/*
 * search_benchmarks.cpp
 *
 * This program benchmarks two search methods (linear vs. binary)
 * on an increasingly large, sorted data set. It computes the average
 * execution time (in microseconds) and the standard deviation across
 * multiple iterations. Results are written to a CSV file.
 *
 * Usage (example):
 *   ./search_benchmarks [--iterations 5] [--maxSize 1000000]
 *
 * By default, it will run 5 iterations for each size in an internal
 * list of sizes (e.g., 1000, 10000, 100000, etc.) up to 1,000,000.
 * Adjust or extend the sizes as needed.
*/

#include "math.h"
#include "timer.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <vector>

/*----------------------------------------------------------------------------*/

using ElementType = long long;
using TimerType = Timer<std::nano>;

constexpr ElementType keyTransform ( ElementType const & _key )
{
    return _key * 0.5;
}

constexpr int ITERATIONS = 30;

static std::vector<ElementType> SIZES =
{
    10, 20, 50,
    100, 200, 500,
    1000, 2000, 5000,
    10000, 20000, 50000,
    100000, 200000, 500000
};

constexpr int SIZE_FACTOR = 1;

constexpr size_t ARRAY_SIZE = 50;

/*----------------------------------------------------------------------------*/

template < typename T >
inline void doNotOptimize ( const T& value )
{
    asm volatile( "" : : "r,m"(value) : "memory" );
}

/*----------------------------------------------------------------------------*/

std::vector<ElementType> generateSortedVector ( ElementType size )
{
    std::vector<ElementType> data(size);
    for (int i = 0; i < size; ++i)
    {
        data[i] = i;
    }
    return data;
}

/*----------------------------------------------------------------------------*/

int linearSearch ( const std::vector<ElementType>& data, ElementType key )
{
    for (int i = 0; i < static_cast<int>(data.size()); ++i)
    {
        if (data[i] == key)
        {
            return i;
        }
    }
    return -1;
}

/*----------------------------------------------------------------------------*/

int binarySearch ( const std::vector<ElementType>& data, ElementType key )
{
    int left = 0;
    int right = static_cast<int>(data.size()) - 1;
    while (left <= right)
    {
        int mid = left + (right - left) / 2;
        if (data[mid] == key)
        {
            return mid;
        }
        else if (data[mid] < key)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }
    return -1;
}

/*----------------------------------------------------------------------------*/

template <typename SearchFunc>
std::vector<double> runBenchmarkVec (
    SearchFunc searchFunction,
    int size,
    int iterations
)
{
    // Generate a sorted vector of the requested size
    std::vector<ElementType> data = generateSortedVector( size );
    // The key is at the very end to ensure worst-case for linear search
    int key = data[size - 1];

    std::vector<double> times;
    times.reserve(iterations);

    for (int i = 0; i < iterations; ++i)
    {
        TimerType timer("Search iteration");
        ElementType result = searchFunction(data, key);
        doNotOptimize(result);
        double duration = timer.stop();
        times.push_back(duration);
    }

    return times;
}

/*----------------------------------------------------------------------------*/

std::vector<double> runBenchmarkSet(
    const std::set<ElementType>& s,
    ElementType key,
    int iterations
)
{
    std::vector<double> times;
    times.reserve(iterations);
    for (int i = 0; i < iterations; ++i)
    {
        TimerType timer("Set lookup iteration");
        auto result = s.find(key);
        doNotOptimize(result);
        double duration = timer.stop();
        times.push_back(duration);
    }
    return times;
}

/*----------------------------------------------------------------------------*/


// Benchmarking linear search on an std::array.
template <size_t N>
std::array<ElementType, N> generateSortedArray ()
{
    std::array<ElementType, N> arr{};
    for (size_t i = 0; i < N; ++i)
    {
        arr[i] = i;
    }
    return arr;
}

template <size_t N>
int linearSearchArray ( const std::array<ElementType, N>& arr, ElementType key )
{
    for (size_t i = 0; i < N; ++i)
    {
        if (arr[i] == key)
            return static_cast<int>(i);
    }
    return -1;
}

// Run benchmark for linear search on an std::array.
std::vector<double> runBenchmarkArrayLinear ( int iterations )
{
    constexpr size_t N = ARRAY_SIZE;
    std::array<ElementType, N> arr = generateSortedArray<N>();
    ElementType key = arr[N - 1]; // Worst-case scenario.
    std::vector<double> times;
    times.reserve(iterations);
    for (int i = 0; i < iterations; ++i)
    {
        TimerType timer("Array linear search iteration");
        int result = linearSearchArray(arr, key);
        doNotOptimize(result);
        double duration = timer.stop();
        times.push_back(duration);
    }
    return times;
}

/*----------------------------------------------------------------------------*/

// Structure to hold benchmark results for different search methods.
struct SearchBenchmarkResults {
    std::vector<double> linearAvg;
    std::vector<double> linearStd;
    std::vector<double> binaryAvg;
    std::vector<double> binaryStd;
    std::vector<double> setLookupAvg;
    std::vector<double> setLookupStd;
};

/*----------------------------------------------------------------------------*/

void writeResultsToCSV (
    const std::string& filename,
    const std::vector<ElementType>& sizes,
    const SearchBenchmarkResults & results
)
{
    std::ofstream ofs( filename );
    if ( !ofs )
    {
        std::cerr
            << "Error: cannot open file " << filename << " for writing.\n"
        ;
        return;
    }

    ofs << "Size,LinearSearchAvg,LinearSearchStd,BinarySearchAvg,BinarySearchStd,SetLookupAvg,SetLookupStd\n";
    for (size_t i = 0; i < sizes.size(); ++i)
    {
        ofs << sizes[i] << ","
            << results.linearAvg[i] << ","
            << results.linearStd[i] << ","
            << results.binaryAvg[i] << ","
            << results.binaryStd[i] << ","
            << results.setLookupAvg[i] << ","
            << results.setLookupStd[i] << "\n";
    }

    ofs.close();
    std::cout << "Benchmark results written to " << filename << std::endl;
}

/*----------------------------------------------------------------------------*/

void runSearchBenchmarks (
    const std::vector<ElementType>& sizes,
    int iterations,
    SearchBenchmarkResults& results
)
{
    // Resize result vectors to match the number of sizes.
    results.linearAvg.resize(sizes.size(), 0.0);
    results.linearStd.resize(sizes.size(), 0.0);
    results.binaryAvg.resize(sizes.size(), 0.0);
    results.binaryStd.resize(sizes.size(), 0.0);
    results.setLookupAvg.resize(sizes.size(), 0.0);
    results.setLookupStd.resize(sizes.size(), 0.0);

    for (size_t i = 0; i < sizes.size(); ++i)
    {
        int size = sizes[i];

        // Prepare data for vector-based searches.
        std::vector<ElementType> data = generateSortedVector(size);

        for ( int i = 0; i < data.size(); ++i )
        {
            doNotOptimize( data[ i ] );
        }

        int key = keyTransform( data.back() );

        // Run linear search benchmark.
        auto linearTimes = runBenchmarkVec(linearSearch, key, iterations);
        double sumLinear = 0.0;
        for (auto t : linearTimes) sumLinear += t;
        double avgLinear = sumLinear / linearTimes.size();
        double stdLinear = computeStdDev(linearTimes, avgLinear);
        results.linearAvg[i] = avgLinear;
        results.linearStd[i] = stdLinear;

        // Run binary search benchmark.
        auto binaryTimes = runBenchmarkVec(binarySearch, key, iterations);
        double sumBinary = 0.0;
        for (auto t : binaryTimes) sumBinary += t;
        double avgBinary = sumBinary / binaryTimes.size();
        double stdBinary = computeStdDev(binaryTimes, avgBinary);
        results.binaryAvg[i] = avgBinary;
        results.binaryStd[i] = stdBinary;

        // Run set lookup
        // Build a std::set from the vector for set lookup.
        std::set<ElementType> s(data.begin(), data.end());
        auto setTimes = runBenchmarkSet(s, key, iterations );
        double sumSet = 0.0;
        for (auto t : setTimes) sumSet += t;
        double avgSet = sumSet / setTimes.size();
        double stdSet = computeStdDev(setTimes, avgSet);
        results.setLookupAvg[i] = avgSet;
        results.setLookupStd[i] = stdSet;

        std::cout << "Size: " << size
                << " | Linear Avg: " << avgLinear << TimerType::unit()
                << " | Set Lookup Avg: " << avgSet << TimerType::unit()
                << " | Binary Avg: " << avgBinary << TimerType::unit() << "\n"
        ;
    }
}

/*----------------------------------------------------------------------------*/

void runArrayBenchmark ( int iterations )
{
    std::cout
        << "\nBenchmarking linear search on std::array (size "
        << ARRAY_SIZE << ")...\n"
    ;
    auto arrayTimes = runBenchmarkArrayLinear(iterations);
    double sumArray = 0.0;
    for (auto t : arrayTimes) { sumArray += t; }
    double avgArray = sumArray / arrayTimes.size();
    double stdArray = computeStdDev(arrayTimes, avgArray);
    std::cout
        << "Array (std::array) linear search benchmark for size " << ARRAY_SIZE
        << " | Avg: " << avgArray << TimerType::unit()
        << " | Std: " << stdArray << TimerType::unit() << "\n"
    ;
}

/*----------------------------------------------------------------------------*/

int main ( int argc, char* argv[] )
{
    auto iterations = ITERATIONS;
    auto sizeFactor = SIZE_FACTOR;

    std::vector< ElementType > sizes = SIZES;


    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--iterations" && (i + 1) < argc)
        {
            iterations = std::stoi(argv[++i]);
        }
        else if (arg == "--maxSize" && (i + 1) < argc)
        {
            int maxSize = std::stoi(argv[++i]);
            sizes.clear();
            int step = maxSize / 10;
            for (int s = step; s <= maxSize; s += step)
            {
                sizes.push_back(s);
            }
        }
        else if (arg == "--factor" && (i + 1) < argc)
        {
            sizeFactor = std::stoi(argv[++i]);
        }
    }

    std::transform(
        sizes.begin(), sizes.end(), sizes.begin(),
        [ & ] ( ElementType x ) { return x * sizeFactor; }
    );


    std::cout << "Running search benchmarks with iterations=" << iterations << "\n";
    std::cout << "Sizes to test: ";
    for (auto s : sizes)
        std::cout << s << " ";
    std::cout << "\n";

    // Create a structure to hold benchmark results.
    SearchBenchmarkResults results;

    // Run the benchmarks.
    runSearchBenchmarks(sizes, iterations, results);

    // Write results to CSV.
    std::string filename = "search_benchmarks.csv";
    writeResultsToCSV(filename, sizes, results);


    runArrayBenchmark( iterations );

    return 0;
}
