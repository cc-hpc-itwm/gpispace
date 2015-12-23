#include <we/type/schedule_data.hpp>

#include <stdexcept>

namespace we
{
  namespace type
  {
    schedule_data::schedule_data
      (const boost::optional<unsigned long>& num_worker)
        : _num_worker (num_worker)
    {
      if (!!_num_worker && _num_worker.get() == 0UL)
      {
        throw std::logic_error ("schedule_data: num_worker == 0UL");
      }
    }

    const boost::optional<unsigned long>& schedule_data::num_worker() const
    {
      return _num_worker;
    }
  }
}
