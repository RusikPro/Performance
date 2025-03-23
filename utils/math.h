#ifndef __UTILS__MATH_H__
#define __UTILS__MATH_H__

/*----------------------------------------------------------------------------*/

#include <cmath>
#include <vector>

/*----------------------------------------------------------------------------*/

// Helper: Calculate average from a vector of times.
double calculateAverage ( std::vector< double > const & times )
{
    double sum = 0.0;
    for ( double t : times )
        sum += t;
    return sum / times.size();
}

// Helper: Calculate standard deviation (if needed).
double calculateStdDev ( std::vector< double>  const & times, double avg )
{
    double sumSq = 0.0;
    for (double t : times) {
        double diff = t - avg;
        sumSq += diff * diff;
    }
    return std::sqrt(sumSq / times.size());
}

/*----------------------------------------------------------------------------*/

#endif // __UTILS__MATH_H__
