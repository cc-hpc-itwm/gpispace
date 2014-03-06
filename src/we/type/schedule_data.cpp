// mirko.rahn@itwm.fraunhofer.de

#include <we/type/schedule_data.hpp>

namespace we
{
  namespace type
  {
    schedule_data::schedule_data()
      : _num_worker (boost::none)
    {}

    schedule_data::schedule_data
      (const boost::optional<unsigned long>& num_worker)
        : _num_worker (num_worker)
    {}

    const boost::optional<unsigned long>& schedule_data::num_worker() const
    {
      return _num_worker;
    }
  }
}
