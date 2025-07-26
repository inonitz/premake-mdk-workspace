#ifndef __UTIL2_TIME_HEADER__
#define __UTIL2_TIME_HEADER__
#include "util2/C/util2_api.h"
#include "util2/C/base_type.h"
#include <chrono>


namespace util2::Time {


using nanosecond  = std::chrono::nanoseconds;
using millisecond = std::chrono::milliseconds;
using timepoint_nano  = std::chrono::time_point<std::chrono::steady_clock, nanosecond>;
using timepoint_milli = std::chrono::time_point<std::chrono::steady_clock, millisecond>;
using dursecondf32 = std::chrono::duration<float>;
using dursecondf64 = std::chrono::duration<double>;


template<
    class clock_t    = std::chrono::steady_clock,
    class result_t   = millisecond,
    class duration_t = millisecond>
auto since(std::chrono::time_point<clock_t, duration_t> const& start) 
{
    return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}

template<class clock_t = std::chrono::steady_clock> auto now() { 
    return clock_t::now(); 
}

template<typename Before, typename After> auto duration_cast(Before const& timepoint) {
    return std::chrono::duration_cast<After>(timepoint);
}

template< typename _From > auto to_milli(_From const& timepoint) {
    return duration_cast<_From, millisecond>(timepoint);
}

template< typename _From > auto to_nano(_From const& timepoint) {
    return duration_cast<_From, nanosecond>(timepoint);
}


template<
    class DT = std::chrono::nanoseconds,
    class ClockT = std::chrono::steady_clock
> class Timer
{
public:
    using timep_t = typename ClockT::time_point;
    using timep_dt = DT;


    void tick() { 
        _end = timep_t{}; 
        _start = ClockT::now(); 
    }    
    void tock() { _end = ClockT::now(); }
    

    template <class T = DT> 
    auto duration() const { 
        static_assert(_end == timep_t{}, "tock before reporting");
        return std::chrono::duration_cast<T>(_end - _start); 
    }
private:
    timep_t _start = ClockT::now(), _end = {};
};


class Timestamp
{
public:
    void begin()
    {
        m_last_copy[0] = m_stamps[0];
        m_last_copy[1] = m_stamps[1];

        m_stamps[0] = Time::now();
        return;
    }
    void end() {
        m_stamps[1] = Time::now();
        return;
    }

    auto curr_value() const {
        return get_underlying_value_choose<diff_type_t, true>();
    }

    auto previous_value() const {
        return get_underlying_value_choose<diff_type_t, false>();
    }

    template<typename T> T value_units(f64 HowManyUnitsIn1Second) const
    {
        f64 unitConvert = 1e-9 * HowManyUnitsIn1Second;
        return static_cast<T>(get_underlying_value_choose<i64, false>() * unitConvert);
    }


private:
    Time::timepoint_nano m_last_copy[2]{};
    Time::timepoint_nano m_stamps[2]{};

    using diff_type_t = decltype(m_last_copy[1] - m_last_copy[0]);


    template<typename T, bool PrevFalseCurrTrue = false> T get_underlying_value_choose() const 
    {
        diff_type_t diff;
        if constexpr (PrevFalseCurrTrue) {
            diff = m_stamps[1] - m_stamps[0];
        } else {
            diff = m_last_copy[1] - m_last_copy[0];
        }

        if constexpr (std::is_same<T, i64>::value) {
            return diff.count();
        } else {
            return diff;
        }
    }
};


/*
	This is just a buffer for keeping track of the sum(samples[]) / length(samples[]),
	constantly calculating averages
*/
template<
    u32 maxSamples
> class SMATick /* Sampling Average Tick */
{
public:

	u64 calcTick(u64 currFrameTick) 
	{
		m_sum -= m_tickbuf[m_currSample];
		m_sum += currFrameTick;
		m_tickbuf[m_currSample] = currFrameTick;
		
		++m_currSample;
		++m_totalSamples;
		m_currSample *= !(m_currSample == maxSamples);
		
        return m_sum / static_cast<u64>(maxSamples);
	}


    /* apparently g++ (10.2.1 20210110) doesn't like this code, so I guess no predefined template instatiation to save up compile time :( */
    // template<> u32 calcTick<u32>(u64 currFrameTick);
    // template<> u64 calcTick<u64>(u64 currFrameTick);
    // template<> f32 calcTick<f32>(u64 currFrameTick);
    // template<> f64 calcTick<f64>(u64 currFrameTick);
private:
	u64 m_totalSamples{0};
	u32 m_currSample{0};
	u64 m_sum{0};
	u64 m_tickbuf[maxSamples]{0};
};





template i64                    Timestamp::get_underlying_value_choose<i64,                    false>() const;
template Timestamp::diff_type_t Timestamp::get_underlying_value_choose<Timestamp::diff_type_t, false>() const;
template i64                    Timestamp::get_underlying_value_choose<i64,                    true>() const;
template Timestamp::diff_type_t Timestamp::get_underlying_value_choose<Timestamp::diff_type_t, true>() const;
template class SMATick<4>;
template class SMATick<6>;
template class SMATick<8>;
template class SMATick<10>;
template class SMATick<16>;
template class SMATick<32>;


#define UTIL2_TIME_NAMESPACE_MEASURE_CODE_BLOCK(counter, ...) \
    { \
        counter.begin(); \
        __VA_ARGS__; \
        counter.end(); \
    } \


#define UTIL2_TIME_NAMESPACE_STATIC_TIMESTAMP_MAXIMUM 16
#define UTIL2_TIME_NAMESPACE_STATIC_TIMESTAMP_MAXIMUM_INDEX (TIME_NAMESPACE_STATIC_TIMESTAMP_MAXIMUM - 1)


UTIL2_API Timestamp& getGeneralPurposeStamp(u8 index);


} /* namespace util2::Time */


#endif /* __UTIL2_TIME_HEADER__ */