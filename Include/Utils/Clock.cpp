#include "Clock.h"

namespace Utils
{
    namespace detail
    {
        class Clock
        {
        public:
            LARGE_INTEGER start;
            LARGE_INTEGER frequency;
        };
    }

    Clock::Clock()
        : _impl(new detail::Clock)
    {
        reset();

        QueryPerformanceFrequency(&_impl->frequency);
    }

    Clock::Clock(const Clock& from)
        : _impl(new detail::Clock(*from._impl))
    {
    }

    Clock& Clock::operator=(const Clock& ref)
    {
        *_impl = *ref._impl;
        return *this;
    }

    Clock::~Clock()
    {
        delete _impl;
    }

    void Clock::reset()
    {
        QueryPerformanceCounter(&_impl->start);
    }

    void Clock::set(const int64_t time)
    {
        reset();
        _impl->start.QuadPart -= static_cast<long long>(time * _impl->frequency.QuadPart / 1000);
    }

    float Clock::getTimef() const
    {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return 1000.0f * (now.QuadPart - _impl->start.QuadPart) / _impl->frequency.QuadPart;
    }

    float Clock::resetTimef()
    {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        const float time = 1000.0f * (now.QuadPart - _impl->start.QuadPart) / _impl->frequency.QuadPart;

        _impl->start = now;
        return time;
    }

    int64_t Clock::getTime64() const
    {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return (1000 * (now.QuadPart - _impl->start.QuadPart) + (_impl->frequency.QuadPart >> 1)) / _impl->frequency.QuadPart;
    }

    double Clock::getTimed() const
    {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return 1000.0 * (now.QuadPart - _impl->start.QuadPart) / _impl->frequency.QuadPart;
    }
	
}