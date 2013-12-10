// mirko.rahn@itwm.fraunhofer.de

#include <we/type/schedule_data.hpp>

namespace we
{
  namespace type
  {
    schedule_data::schedule_data()
      : _num_worker (boost::none)
      , _vmem (boost::none)
    {}

    schedule_data::schedule_data
      ( const boost::optional<long>& num_worker
      , const boost::optional<long>& vmem
      )
        : _num_worker (num_worker)
        , _vmem (vmem)
    {}

    const boost::optional<long>& schedule_data::num_worker() const
    {
      return _num_worker;
    }
    const boost::optional<long>& schedule_data::vmem() const
    {
      return _vmem;
    }
  }
}
