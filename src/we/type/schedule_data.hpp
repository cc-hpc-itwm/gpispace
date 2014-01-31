// mirko.rahn@itwm.fraunhofer.de

#ifndef FHG_PNET_WE_TYPE_SCHEDULE_DATA_HPP
#define FHG_PNET_WE_TYPE_SCHEDULE_DATA_HPP

#include <boost/optional.hpp>

namespace we
{
  namespace type
  {
    struct schedule_data
    {
    public:
      schedule_data();
      schedule_data ( const boost::optional<unsigned long>& num_worker
                    , const boost::optional<unsigned long>& vmem
                    );

      const boost::optional<unsigned long>& num_worker() const;
      const boost::optional<unsigned long>& vmem() const;

    private:
      const boost::optional<unsigned long> _num_worker;
      const boost::optional<unsigned long> _vmem;
    };
  }
}

#endif
