#include "../utils/benchmark.hpp"

#include <cmath>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <string>

/*----------------------------------------------------------------------------*/

using DataType = double;
using TimerType = std::milli;

template < size_t DummySize >
struct Entity {
    DataType x, y;
    DataType vx, vy;
    DataType dummy[DummySize];

    void update ( DataType dt )
    {
        x += vx * dt;
        y += vy * dt;
        x *= 1.000001;
        y *= 1.000001;
    }
};

/*----------------------------------------------------------------------------*/

template < size_t DummySize >
void
benchmarkEntity ( const std::string & label, size_t count, size_t iterations )
{
    std::vector< Entity< DummySize > > entities( count );

    for ( std::size_t i = 0; i < count; ++i )
    {
        entities[i].x = static_cast<DataType>(rand()) / RAND_MAX;
        entities[i].y = static_cast<DataType>(rand()) / RAND_MAX;
        entities[i].vx = static_cast<DataType>(rand()) / RAND_MAX;
        entities[i].vy = static_cast<DataType>(rand()) / RAND_MAX;
    }

    auto benchFunc = [ &entities, count ] () -> DataType {
        volatile DataType accumulator = 0;
        for ( std::size_t i = 0; i < count; ++i )
        {
            entities[i].update(0.016); // Assume dt ~ 16ms (60fps)
            accumulator += entities[i].x;
        }
        return accumulator;
    };

    auto iterationTimes = runBenchmarkSingle< TimerType >(
        iterations, benchFunc
    );

    // Calculate average and standard deviation.
    double average = calculateAverage( iterationTimes );
    double stddev = calculateStdDev( iterationTimes, average );

    std::cout << label << " Benchmark: "
            << "Average time: " << average << " " << Timer< TimerType >::unit()
            << ", StdDev: " << stddev << " " << Timer< TimerType >::unit()
            << std::endl
    ;
}

/*----------------------------------------------------------------------------*/

// Flush the cache by reading/writing a large memory block.
void flushCache()
{
    // Allocate a buffer that's much larger than the L1/L2/L3 caches.
    // Adjust the size as needed (here we use 50 MB).
    const size_t size = 50 * 1024 * 1024;
    volatile char* buffer = new volatile char[size];

    // Touch the buffer in steps (cache line size is usually 64 bytes).
    for (size_t i = 0; i < size; i += 1) {
        buffer[i] = static_cast<char>(i % 256);
    }

    // Prevent the compiler from optimizing the loop away.
    asm volatile("" : : "r"(buffer) : "memory");

    // Clean up.
    delete[] buffer;
}

/*----------------------------------------------------------------------------*/

int main ()
{
    size_t count = 100000;
    size_t iterations = 30;

    // Run benchmarks with different dummy sizes.
    flushCache();
    benchmarkEntity<16>("DummySize 16", count, iterations);
    flushCache();
    benchmarkEntity<32>("DummySize 32", count, iterations);
    flushCache();
    benchmarkEntity<64>("DummySize 64", count, iterations);
    flushCache();
    benchmarkEntity<128>("DummySize 128", count, iterations);
    flushCache();
    benchmarkEntity<256>("DummySize 256", count, iterations);

    return 0;
}

/*----------------------------------------------------------------------------*/
