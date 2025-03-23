#ifndef __UTILS__TIMER_H__
#define __UTILS__TIMER_H__

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
    static constexpr auto value = "ns";
};

template <>
struct ratio_repr< std::milli >
{
    static constexpr auto value = "ms";
};

template <>
struct ratio_repr< std::micro >
{
    static constexpr auto value = "us";
};

} // namespace

/*----------------------------------------------------------------------------*/

template < typename _RatioT = std::micro >
class Timer {

public:

    using Ratio = _RatioT;
    using Clock = std::chrono::high_resolution_clock;
    using CurrentClock = decltype( std::chrono::high_resolution_clock::now() );
    using Duration = std::chrono::duration< double, _RatioT >;

    const char* getUnit () const
    {
        return ratio_repr<_RatioT>::value;
    }

    static const char* unit ()
    {
        return ratio_repr<_RatioT>::value;
    }

    Timer ( std::string const & _title )
        :   m_title( _title )
        ,   m_start( Clock::now() )
        ,   m_stopped( false )
    {
    }

    double stop ()
    {
        if ( m_stopped ) return 0;

        auto end = Clock::now();
        m_duration = end - m_start;
        m_stopped = true;
        // std::cout   << m_title << " took " << m_duration.count()
        //             << " " << ratio_repr< _RatioT >::value << std::endl
        // ;
        return m_duration.count();
    }

    ~Timer ()
    {
        if ( m_stopped ) return;

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

    std::string m_title;
    const CurrentClock m_start;
    Duration m_duration{ 0 };
    bool m_stopped;
};

/*----------------------------------------------------------------------------*/

#endif  //  __UTILS__TIMER_H__
