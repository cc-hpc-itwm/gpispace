// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <chrono>
#include <limits>
#include <string>

namespace fhg
{
  namespace util
  {
    class statistic
    {
    public:
      statistic (std::string const& description)
        : _description (description)
        , _min (std::numeric_limits<double>::max())
        , _max (0.0)
        , _sum (0.0)
        , _sqsum (0.0)
        , _count (0)
      {}

      std::chrono::high_resolution_clock::rep now() const
      {
        return std::chrono::duration_cast<std::chrono::microseconds>
          (std::chrono::high_resolution_clock().now().time_since_epoch())
          .count();
      }

      void tick (std::chrono::high_resolution_clock::rep _delta)
      {
        const double delta (static_cast<double> (_delta) / 1000.0);
        _min = (delta < _min) ? delta : _min;
        _max = (delta > _max) ? delta : _max;
        _sum += delta;
        _sqsum += delta * delta;
        _count += 1;
      }

      double min() const
      {
        return _min;
      }
      double max() const
      {
        return _max;
      }
      double sum() const
      {
        return _sum;
      }
      double sqsum() const
      {
        return _sqsum;
      }
      unsigned long count() const
      {
        return _count;
      }

    private:
      std::string const _description;
      double _min;
      double _max;
      double _sum;
      double _sqsum;
      unsigned long _count;
    };
  }
}
