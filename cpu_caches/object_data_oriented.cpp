#include "../utils/benchmark.hpp"

#include <iostream>
#include <vector>
#include <cstdlib>

/*----------------------------------------------------------------------------*/

using TimerType = Timer< std::milli >;
using DataType = double;

constexpr int DUMMY_SIZE = 16;

// Object oriented design: each entity encapsulates its data and update method.
struct Entity
{
    DataType x, y;
    DataType vx, vy;
    DataType dummy[ DUMMY_SIZE ];  // Extra data that is not used in the update.

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
        std::cout, "Object Oriented", iterationsTimes[ 0 ]
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
        std::cout, "Object Oriented", iterationsTimes[ 0 ]
    );
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void benchmarkObjectOrientedSizes(size_t baseCount, size_t iterations,
    int globalStart, int globalEnd, int globalStep,
    std::ostream & out)
{
    // preCalc: Create a vector of Entities with a size that grows
    // with globalIndex.
    auto preCalc = [ baseCount ]( int globalIndex ) -> std::vector<Entity> {
        int newSize = static_cast<int>( baseCount ) * ( globalIndex + 1 );
        std::vector< Entity > entities(newSize);
        for (size_t i = 0; i < entities.size(); ++i)
        {
            entities[i].x = static_cast<DataType>(rand()) / RAND_MAX;
            entities[i].y = static_cast<DataType>(rand()) / RAND_MAX;
            entities[i].vx = static_cast<DataType>(rand()) / RAND_MAX;
            entities[i].vy = static_cast<DataType>(rand()) / RAND_MAX;
        }
        return entities;
    };

    // benchFunc: Process the vector of Entities.
    // Note: We accept the vector by value (a copy) so that each iteration
    // uses fresh data.
    auto benchFunc =
        [] ( std::vector< Entity > entities, int globalIndex ) -> DataType
    {
        volatile DataType accumulator = 0;
        for ( std::size_t i = 0; i < entities.size(); ++i )
        {
            entities[i].update(0.016f); // Assume dt ~ 16ms (60fps)
            accumulator += entities[i].x;
        }
        return accumulator;
    };

    // Run the benchmark for each global iteration (each representing a
    // different array size).
    auto rawMeasuremenetsList = runBenchmarkWithPreCalc< TimerType::Ratio >(
        globalStart, globalEnd, globalStep, iterations,
        preCalc, benchFunc
    );

    // Print the list of averages.
    printBenchmarkStatsList< TimerType >(
        out, "Object Oriented (Growing Array Sizes)", rawMeasuremenetsList
    );
}

/*----------------------------------------------------------------------------*/

// Data-oriented structure to hold arrays.
struct DataArrays
{
    std::vector<DataType> posX;
    std::vector<DataType> posY;
    std::vector<DataType> velX;
    std::vector<DataType> velY;
};

void benchmarkDataOrientedSizes (
    size_t baseCount, size_t iterations,
    int globalStart, int globalEnd, int globalStep,
    std::ostream & out
)
{
    // preCalc: Create and initialize DataArrays whose size grows with the global index.
    auto preCalc = [ baseCount ](int globalIndex) -> DataArrays {
        int newSize = static_cast<int>(baseCount) * (globalIndex + 1);
        DataArrays arrays;
        arrays.posX.resize(newSize);
        arrays.posY.resize(newSize);
        arrays.velX.resize(newSize);
        arrays.velY.resize(newSize);
        for (int i = 0; i < newSize; ++i)
        {
            arrays.posX[i] = static_cast<DataType>(rand()) / RAND_MAX;
            arrays.posY[i] = static_cast<DataType>(rand()) / RAND_MAX;
            arrays.velX[i] = static_cast<DataType>(rand()) / RAND_MAX;
            arrays.velY[i] = static_cast<DataType>(rand()) / RAND_MAX;
        }
        return arrays;
    };

    // benchFunc: Process the DataArrays by updating each element and
    // accumulating a value.
    auto benchFunc = [] ( DataArrays arrays, int /*globalIndex*/ ) -> DataType
    {
        volatile DataType accumulator = 0;
        int size = arrays.posX.size();
        for (int i = 0; i < size; ++i)
        {
            arrays.posX[i] += arrays.velX[i] * 0.016f;
            arrays.posY[i] += arrays.velY[i] * 0.016f;
            arrays.posX[i] *= 1.000001;
            arrays.posY[i] *= 1.000001;
            accumulator += arrays.posX[i];
        }
        return accumulator;
    };

    // Run the benchmark over a range of global iterations.
    auto rawMeaduremenetsList = runBenchmarkWithPreCalc< TimerType::Ratio >(
        globalStart, globalEnd, globalStep, iterations,
        preCalc, benchFunc
    );

    // Print the list of averages.
    printBenchmarkStatsList< TimerType >(
        out, "Data Oriented (Growing Array Sizes)", rawMeaduremenetsList
    );
}

/*----------------------------------------------------------------------------*/


int main ()
{
    const size_t count = 1000000;
    const size_t iterations = 30;

    std::cout << "Size of Entity: " << sizeof(Entity) << " bytes" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout
        << "Benchmarking with " << count << " elements and "
        << iterations << " iterations per test." << std::endl
    ;
    std::cout << "========================================" << std::endl
        << std::endl
    ;

    std::cout << ">> Running Object Oriented Benchmark..." << std::endl;
    benchmarkObjectOriented(count, iterations);
    std::cout << std::endl;

    std::cout << ">> Running Data Oriented Benchmark..." << std::endl;
    benchmarkDataOriented(count, iterations);
    std::cout << std::endl;

    std::cout << "========================================" << std::endl
        << std::endl
    ;

    // Global parameter range: for example, test with increasing array sizes.
    int globalStart = 0;
    // This will test 5 different sizes (0, 1, 2, 3, and 4 scaling factors)
    int globalEnd = 4;
    int globalStep = 1;

    std::cout
        << ">> Running Object Oriented Benchmark with Growing Array Sizes:"
        << std::endl
    ;
    benchmarkObjectOrientedSizes(
        count, iterations, globalStart, globalEnd, globalStep, std::cout
    );
    std::cout << std::endl;

    std::cout
        << ">> Running Data Oriented Benchmark with Growing Array Sizes:"
        << std::endl
    ;
    benchmarkDataOrientedSizes(
        count, iterations, globalStart, globalEnd, globalStep, std::cout
    );
    std::cout << std::endl;

    std::cout << "Benchmarking complete." << std::endl;

    return 0;
}

/*----------------------------------------------------------------------------*/
