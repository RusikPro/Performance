#include "../utils/benchmark.hpp"

#include <iostream>
#include <vector>
#include <cstdlib>

/*----------------------------------------------------------------------------*/

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

/*----------------------------------------------------------------------------*/

void benchmarkObjectOriented ( size_t count, size_t iterations )
{
    std::vector<Entity> entities(count);

    for (size_t i = 0; i < count; ++i) {
        entities[i].x = static_cast<DataType>(rand()) / RAND_MAX;
        entities[i].y = static_cast<DataType>(rand()) / RAND_MAX;
        entities[i].vx = static_cast<DataType>(rand()) / RAND_MAX;
        entities[i].vy = static_cast<DataType>(rand()) / RAND_MAX;
    }

    auto benchFunc = [ &entities, count ] ( int i ) -> DataType {
        int elements = count + count * i;
        volatile DataType accumulator = 0;
        for ( std::size_t i = 0; i < elements; ++i )
        {
            entities[i].update( 0.016f ); // Assume dt ~ 16ms (60fps)
            accumulator += entities[i].x;
        }
        return accumulator;
    };

    auto iterationsTimes = runBenchmark< TimerType::Ratio >(
        0, 0, 1, iterations, benchFunc
    );

    printBenchmarkStats< TimerType >(
        std::cout, "Object Oriented", iterationsTimes
    );
}

/*----------------------------------------------------------------------------*/

void benchmarkDataOriented ( size_t count, size_t iterations )
{
    std::vector<DataType> posX(count);
    std::vector<DataType> posY(count);
    std::vector<DataType> velX(count);
    std::vector<DataType> velY(count);

    for ( std::size_t i = 0; i < count; ++i )
    {
        posX[i] = static_cast<DataType>(rand()) / RAND_MAX;
        posY[i] = static_cast<DataType>(rand()) / RAND_MAX;
        velX[i] = static_cast<DataType>(rand()) / RAND_MAX;
        velY[i] = static_cast<DataType>(rand()) / RAND_MAX;
    }

    auto benchFunc = [ &posX, &posY, &velX, &velY, count ] ( int ) -> DataType
    {
        volatile DataType accumulator = 0;
        for ( std::size_t i = 0; i < count; ++i )
        {
            posX[i] += velX[i] * 0.016f;
            posY[i] += velY[i] * 0.016f;
            posX[i] *= 1.000001;
            posY[i] *= 1.000001;
            accumulator += posX[i];
        }
        return accumulator;
    };

    auto iterationsTimes = runBenchmark< TimerType::Ratio >(
        0, 0, 1, iterations, benchFunc
    );

    printBenchmarkStats< TimerType >(
        std::cout, "Object Oriented", iterationsTimes
    );
}

/*----------------------------------------------------------------------------*/

int main ()
{
    const size_t count = 1000000;
    const size_t iterations = 100;

    benchmarkObjectOriented(count, iterations);
    benchmarkDataOriented(count, iterations);

    std::cout << sizeof( Entity ) << std::endl;

    return 0;
}

/*----------------------------------------------------------------------------*/
