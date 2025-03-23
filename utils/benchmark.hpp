#ifndef __UTILS__BENCHMARKS_HPP__
#define __UTILS__BENCHMARKS_HPP__

/*----------------------------------------------------------------------------*/

#include "timer.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

/*----------------------------------------------------------------------------*/

// Helper: Prevent optimization.
template < typename T >
inline void doNotOptimize ( T const & value )
{
    asm volatile("" : : "r,m"(value) : "memory");
}

/*----------------------------------------------------------------------------*/

// Helper: Calculate average from a vector of times.
double calculateAverage ( std::vector<double> const & times )
{
    double sum = 0.0;
    for ( double t : times )
        sum += t;
    return sum / times.size();
}

// Helper: Calculate standard deviation (if needed).
double calculateStdDev ( std::vector<double> const & times, double avg )
{
    double sumSq = 0.0;
    for (double t : times) {
        double diff = t - avg;
        sumSq += diff * diff;
    }
    return std::sqrt(sumSq / times.size());
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
Generalized benchmark function template (single parameter).

Parameters:
- iterations: How many iterations to perform.
- func: A callback function to benchmark.
- args...: Additional arguments passed to the callback function.
*/
template <
        typename _RatioT
    ,   typename _BenchmarkFuncT
    ,   typename... _ArgsT
>
std::vector<double> runBenchmarkSingle (
    int iterations, _BenchmarkFuncT func, _ArgsT&&... args
)
{
    // volatile decltype( func( std::forward< _ArgsT >( args )... ) ) var = 0;
    std::vector<double> iterationTimes;
    iterationTimes.reserve( iterations );
    for (int iter = 0; iter < iterations; ++iter)
    {
        Timer< _RatioT > timer( "Benchmark" );
        auto result = func( std::forward< _ArgsT >( args )... );
        doNotOptimize( result );
        // var = result;
        iterationTimes.push_back(timer.stop());
    }
    return iterationTimes;
}

/*----------------------------------------------------------------------------*/

/*
Generalized benchmark function template (with range parameters).

Parameters:
- paramStart, paramEnd, paramStep: Define the range of the parameter to vary.
    For instance, if you are benchmarking varying number of threads, these could
    be 1 to maxThreads.
- iterations: How many inner iterations to perform per parameter value.
- func: A callback function to benchmark. The first parameter of func is the
        current parameter value.
- args...: Additional arguments passed to the callback function.

The Timer type uses Ratio (for example, std::milli or std::micro) as its time
unit.
*/
template <
        typename _RatioT
    ,   typename _BenchmarkFuncT
    ,   typename... _ArgsT
>
std::vector<double> runBenchmark (
    int paramStart, int paramEnd, int paramStep, int iterations,
    _BenchmarkFuncT func, _ArgsT &&... args
)
{
    std::vector< double > averages;
    for ( int param = paramStart; param <= paramEnd; param += paramStep )
    {
        std::vector< double > iterationTimes;
        iterationTimes.reserve( iterations );
        for (int iter = 0; iter < iterations; ++iter)
        {
            Timer< _RatioT > timer( "Benchmark" );
            // The benchmark function is called with the current parameter value
            // and any additional parameters.
            auto result = func( param, std::forward< _ArgsT >( args )... );
            doNotOptimize(result);
            iterationTimes.push_back(timer.stop());
        }
        double avg = calculateAverage( iterationTimes );
        averages.push_back(avg);
    }
    return averages;
}

/*----------------------------------------------------------------------------*/

/*
Overloaded version that includes a pre-calculation phase.
The preCalc function is called once per parameter value and its result is passed
to the benchmark function.
Parameters:
- preCalc: A function that takes the parameter value (and additional arguments)
    and returns pre-calculated data.
- func: The benchmark function that will be measured; it takes the
    pre-calculated data, the parameter value, and any additional arguments.
*/
template <
        typename _RatioT
    ,   typename _PreCalcFuncT
    ,   typename _BenchmarkFuncT
    ,   typename... _ArgsT
>
std::vector<double> runBenchmarkWithPreCalc(
    int paramStart, int paramEnd, int paramStep, int iterations,
    _PreCalcFuncT preCalc, _BenchmarkFuncT func, _ArgsT &&... args
)
{
    std::vector< double > averages;
    for ( int param = paramStart; param <= paramEnd; param += paramStep )
    {
        // Pre-calculate data based on the current parameter value.
        auto preCalcData = preCalc(param, std::forward< _ArgsT >(args)...);
        std::vector< double > iterationTimes;
        iterationTimes.reserve( iterations );
        for (int iter = 0; iter < iterations; ++iter)
        {
            Timer< _RatioT > timer( "BenchmarkWithPreCalc" );
            // Pass both the pre-calculated data and the current parameter value
            // to the benchmark function.
            auto result = func(
                preCalcData, param, std::forward< _ArgsT >(args)...
            );
            doNotOptimize(result);
            iterationTimes.push_back(timer.stop());
        }
        double avg = calculateAverage( iterationTimes );
        averages.push_back(avg);
    }
    return averages;
}

/*----------------------------------------------------------------------------*/

/*
Example usage:

Assume you have a function generateMatrix that creates a matrix given rows and
cols, and a benchmark function that accepts the generated matrix along with some
parameters. For example:

auto benchFunc = [](
    int numThreads, int threshold, int rows, int col
) -> long long {
    auto matrix = generateMatrix(rows, cols);
    return countWithContainer(matrix, threshold, numThreads);
};

Then you could call:
std::vector<double> averages = runBenchmark<std::micro>(
    1, maxThreads, 1, iterations,
    benchFunc, threshold, rows, cols);

Similarly, for the version with a pre-calculation step, your preCalc function
might prepare a matrix that will then be used in the benchmark function.
*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

template < typename _TimerT >
void printBenchmarkStats (
        std::ostream & out
    ,   std::string const & label
    ,   std::vector< double > const & iterationTimes
)
{
    double average = calculateAverage( iterationTimes );
    double stddev = calculateStdDev( iterationTimes, average );
    out << label << " Benchmark: "
        << "Average time: " << average << " " << _TimerT::unit()
        << ", StdDev: " << stddev << " " << _TimerT::unit()
        << std::endl;
}

/*----------------------------------------------------------------------------*/

template < typename _TimerT >
void printBenchmarkStatsList (
        std::ostream & out
    ,   std::string const & label
    ,   std::vector< double > const & averages
)
{
    out << label << " Benchmark Averages:" << std::endl;
    for ( std::size_t i = 0; i < averages.size(); ++i )
    {
        out << "Iteration " << i << ": " << averages[i] << " "
            << _TimerT::unit() << std::endl
        ;
    }
}

/*----------------------------------------------------------------------------*/

#endif // __UTILS__BENCHMARKS_HPP__
