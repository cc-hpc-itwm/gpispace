// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_BASE_HPP
#define _FHG_PNETE_DATA_HANDLE_BASE_HPP 1

#include <pnete/data/handle/base.fwd.hpp>

#include <pnete/data/change_manager.fwd.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        class base
        {
        public:
          base (change_manager_t& change_manager );
          virtual ~base() { }

        protected:
          change_manager_t& change_manager() const;

        private:
          change_manager_t& _change_manager;
        };
      }
    }
  }
}

#endif
