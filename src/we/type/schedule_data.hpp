#pragma once

#include <boost/optional.hpp>

namespace we
{
  namespace type
  {
    struct schedule_data
    {
    public:
      schedule_data() = default;
      schedule_data (const boost::optional<unsigned long>& num_worker);

      const boost::optional<unsigned long>& num_worker() const;

    private:
      const boost::optional<unsigned long> _num_worker;
    };
  }
}
