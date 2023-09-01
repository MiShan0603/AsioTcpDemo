#pragma once

#include <windows.h>
#include <vector>
#include <string>

namespace Utils
{
    namespace detail
    {
        class Clock;
    }

    class Clock
    {
    public:
        /** Construct a new clock. @version 1.0 */
        Clock();

        /** Copy-construct a new clock with the same start time . @version 1.0 */
        Clock(const Clock& from);

        /** Destroy the clock. @version 1.0 */
        ~Clock();

        /** Assignment operator. @version 1.7.2 */
        Clock& operator=(const Clock& ref);

        /**
         * Reset the base time of the clock to the current time.
         */
        void reset();

        /** Set the current time of the clock. @version 1.0 */
        void set(const int64_t time);

        /**
         * @return the elapsed time in milliseconds since the last clock reset.
         */
        float getTimef() const;

        /**
         * @return the elapsed time in milliseconds since the last clock reset
         *         and atomically reset the clock.
         */
        float resetTimef();

        /**
         * @return the elapsed time in milliseconds since the last clock reset.
         */
        int64_t getTime64() const;

        /**
         * @return the elapsed time in milliseconds since the last clock reset.
         */
        double getTimed() const;

    private:
        detail::Clock* const _impl;
    };
};
