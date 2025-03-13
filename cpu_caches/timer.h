#ifndef __TIMER_H__
#define __TIMER_H__

/*----------------------------------------------------------------------------*/

#include <chrono>
#include <iostream>
#include <string>

/*----------------------------------------------------------------------------*/

namespace {

template < typename _RatioT >
struct ratio_repr
{
    static constexpr auto value = "unknown";
};

template <>
struct ratio_repr< std::nano >
{
    static constexpr auto value = "nanoseconds";
};

template <>
struct ratio_repr< std::milli >
{
    static constexpr auto value = "milliseconds";
};

template <>
struct ratio_repr< std::micro >
{
    static constexpr auto value = "microseconds";
};

} // namespace

/*----------------------------------------------------------------------------*/

template < typename _RatioT = std::micro >
class Timer {

public:

    using Clock = std::chrono::high_resolution_clock;
    using CurrentClock = decltype( std::chrono::high_resolution_clock::now() );

    Timer ( std::string const & _title )
        :   m_title( _title )
        ,   m_start( Clock::now() )
    {
    }

    ~Timer ()
    {
        std::chrono::duration< double, _RatioT > duration =
            Clock::now() - m_start
        ;
        std::cout
            <<  m_title << " took " << duration.count() << " "
            <<  ratio_repr< _RatioT >::value
            <<  std::endl
        ;
    }

private:

    const std::string m_title;

    const CurrentClock m_start;
};

/*----------------------------------------------------------------------------*/

#endif  //  __TIMER_H__
