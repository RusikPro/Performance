#include "timer.h"

#include <iostream>
#include <vector>
#include <cstdlib>

using TimerType = Timer< std::milli >;
using DataType = double;

// Object oriented design: each entity encapsulates its data and update method.
struct Entity
{
    DataType x, y;
    DataType vx, vy;
    DataType dummy[16];  // Extra data that is not used in the update.

    void update(DataType dt)
    {
        x += vx * dt;
        y += vy * dt;
        x *= 1.000001;
        y *= 1.000001;
    }
};

void benchmarkObjectOriented ( size_t count, size_t iterations )
{
    std::vector<Entity> entities(count);

    // Initialize entities with random values.
    for (size_t i = 0; i < count; ++i) {
        entities[i].x = static_cast<DataType>(rand()) / RAND_MAX;
        entities[i].y = static_cast<DataType>(rand()) / RAND_MAX;
        entities[i].vx = static_cast<DataType>(rand()) / RAND_MAX;
        entities[i].vy = static_cast<DataType>(rand()) / RAND_MAX;
    }

    // Use a volatile accumulator to ensure the work isn't optimized away.
    volatile DataType accumulator = 0;

    double elapsed = 0.0;
    {
        TimerType timer("Object Oriented Update");
        for (size_t iter = 0; iter < iterations; ++iter) {
            for (size_t i = 0; i < count; ++i)
            {
                entities[i].update(0.016f);  // Assume dt ~ 16ms (60fps)
                accumulator += entities[i].x;
            }
        }
        elapsed = timer.stop();
    }

    std::cout
        << "Object Oriented Benchmark: " << elapsed << " "
        << TimerType::unit() << std::endl
    ;
    std::cout << "OO Accumulator: " << accumulator << std::endl;
}

void benchmarkDataOriented ( size_t count, size_t iterations )
{
    std::vector<DataType> posX(count);
    std::vector<DataType> posY(count);
    std::vector<DataType> velX(count);
    std::vector<DataType> velY(count);

    // Initialize arrays with random values.
    for (size_t i = 0; i < count; ++i) {
        posX[i] = static_cast<DataType>(rand()) / RAND_MAX;
        posY[i] = static_cast<DataType>(rand()) / RAND_MAX;
        velX[i] = static_cast<DataType>(rand()) / RAND_MAX;
        velY[i] = static_cast<DataType>(rand()) / RAND_MAX;
    }

    volatile DataType accumulator = 0;
    double elapsed = 0.0;
    {
        TimerType timer("Data Oriented Update");
        for (size_t iter = 0; iter < iterations; ++iter) {
            for (size_t i = 0; i < count; ++i) {
                posX[i] += velX[i] * 0.016f;
                posY[i] += velY[i] * 0.016f;
                posX[i] *= 1.000001;
                posY[i] *= 1.000001;
                accumulator += posX[i];
            }
        }
        elapsed = timer.stop();
    }
    std::cout
        << "Data Oriented Benchmark: " << elapsed << " " << TimerType::unit()
        << std::endl
    ;
    std::cout << "DO Accumulator: " << accumulator << std::endl;
}

int main() {
    const size_t count = 1000000;
    const size_t iterations = 100;

    benchmarkObjectOriented(count, iterations);
    benchmarkDataOriented(count, iterations);

    return 0;
}
