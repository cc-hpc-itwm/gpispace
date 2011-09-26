// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_DATA_CHANGE_MANAGER_HPP
#define _PNETE_DATA_CHANGE_MANAGER_HPP 1

#include <boost/shared_ptr.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      class internal;

      class change_manager
      {
      public:
        change_manager (internal &);

      private:
        internal & _internal;
      };
    }
  }
}

#endif
