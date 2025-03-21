#include "timer.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <string>

/*----------------------------------------------------------------------------*/

using DataType = double;

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

double calculateAverage ( const std::vector<double>& times )
{
    double sum = 0.0;
    for (auto t : times)
        sum += t;
    return sum / times.size();
}

double calculateStdDev ( const std::vector<double>& times, double avg )
{
    double sumSq = 0.0;
    for (auto t : times) {
        double diff = t - avg;
        sumSq += diff * diff;
    }
    return std::sqrt(sumSq / times.size());
}

/*----------------------------------------------------------------------------*/

template < size_t DummySize >
void
benchmarkEntity ( const std::string & label, size_t count, size_t iterations )
{
    std::vector<Entity<DummySize>> entities(count);

    for ( std::size_t i = 0; i < count; ++i )
    {
        entities[i].x = static_cast<DataType>(rand()) / RAND_MAX;
        entities[i].y = static_cast<DataType>(rand()) / RAND_MAX;
        entities[i].vx = static_cast<DataType>(rand()) / RAND_MAX;
        entities[i].vy = static_cast<DataType>(rand()) / RAND_MAX;
    }

    std::vector<double> iterationTimes;
    iterationTimes.reserve(iterations);

    // Use a volatile accumulator to ensure the work isn't optimized away.
    volatile DataType accumulator = 0;


    for ( std::size_t iter = 0; iter < iterations; ++iter )
    {
        Timer< std::milli > timer( label );
        for (size_t i = 0; i < count; ++i) {
            entities[i].update(0.016); // Assume dt ~ 16ms (60fps)
            accumulator += entities[i].x;
        }
        iterationTimes.push_back(timer.stop());
    }

    // Calculate average and standard deviation.
    double average = calculateAverage(iterationTimes);
    double stddev = calculateStdDev(iterationTimes, average);

    std::cout << label << " Benchmark: "
            << "Average time: " << average << " " << Timer< std::milli >::unit()
            << ", StdDev: " << stddev << " " << Timer< std::milli >::unit()
            << std::endl
    ;
}

/*----------------------------------------------------------------------------*/

int main ()
{
    size_t count = 100000;
    size_t iterations = 20;

    // Run benchmarks with different dummy sizes.
    benchmarkEntity<16>("DummySize 16", count, iterations);
    benchmarkEntity<32>("DummySize 32", count, iterations);
    benchmarkEntity<64>("DummySize 64", count, iterations);
    benchmarkEntity<128>("DummySize 128", count, iterations);
    benchmarkEntity<256>("DummySize 256", count, iterations);

    return 0;
}

/*----------------------------------------------------------------------------*/
