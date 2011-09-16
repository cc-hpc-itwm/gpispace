// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_DATA_UNDO_REDO_MANAGER_HPP
#define _PNETE_DATA_UNDO_REDO_MANAGER_HPP 1

#include <boost/shared_ptr.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      class internal;

      class undo_redo_manager
      {
      private:
        internal & _internal;

      public:
        typedef boost::shared_ptr<internal> ptr;

        undo_redo_manager (internal &);
      };
    }
  }
}

#endif
