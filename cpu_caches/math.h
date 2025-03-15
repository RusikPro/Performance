#ifndef __CPU_CACHES__MATH_H__
#define __CPU_CACHES__MATH_H__

#include <vector>

/*----------------------------------------------------------------------------*/
//  Compute standard deviation given a vector of values and their mean
/*----------------------------------------------------------------------------*/
double computeStdDev(const std::vector<double>& values, double mean)
{
    double sumSq = 0.0;
    for (double v : values)
    {
        double diff = v - mean;
        sumSq += diff * diff;
    }
    return std::sqrt(sumSq / values.size());
}

/*----------------------------------------------------------------------------*/

#endif // __CPU_CACHES__MATH_H__
