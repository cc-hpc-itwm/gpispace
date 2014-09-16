#ifndef FHG_UTIL_MEASURE_AVERAGE_TIME_HPP
#define FHG_UTIL_MEASURE_AVERAGE_TIME_HPP

#include <chrono>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    template <typename Duration, typename Func>
      Duration measure_average_time (Func fun, std::size_t const count)
    {
      if (0 == count)
      {
        throw std::invalid_argument ("count has to be positive");
      }

      const std::chrono::high_resolution_clock::time_point start
        (std::chrono::high_resolution_clock::now());

      for (std::size_t i = 0; i < count; ++i)
      {
        fun();
      }

      const Duration time_span
        ( std::chrono::duration_cast<Duration>
          (std::chrono::high_resolution_clock::now() - start)
        );

      return time_span / count;
    }
  }
}

#endif
