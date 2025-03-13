#include "timer.h"

#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <algorithm>

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
    int numRows = static_cast<int>(matrix.size());
    std::atomic<long long> globalCount(0);
    std::vector<std::thread> threads;
    threads.reserve( numThreads );

    std::vector< int > result( numThreads, 0 );

    // Determine chunk size (round up)
    int chunkSize = (numRows + numThreads - 1) / numThreads;

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
            globalCount.fetch_add(localCount, std::memory_order_relaxed);

            // result[ t ] = localCount;
        } );
    }

    for ( auto& th : threads )
    {
        th.join();
    }

    // long long sum = 0;
    // for ( auto c: result )
    // {
    //     sum += c;
    // }
    // return sum;

    return globalCount.load();
}

int main()
{
    const int rows = 10000;
    const int cols = 10000;
    const int numThreads = 16;
    const int threshold = 128;

    Matrix image = generateMatrix( rows, cols );

    {
        Timer< std::micro > timer( "countWithContainer" );
        long long count1 = countWithContainer(image, threshold, numThreads);
    }

    {
        Timer< std::micro > timer( "countWithLocalCounter" );
        long long count1 = countWithLocalCounter(image, threshold, numThreads);
    }

    return 0;
}
