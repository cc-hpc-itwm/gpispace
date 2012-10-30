// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_BASE_HPP
#define _FHG_PNETE_DATA_HANDLE_BASE_HPP 1

#include <pnete/data/handle/base.fwd.hpp>

#include <pnete/data/change_manager.fwd.hpp>

#include <we/type/property.fwd.hpp>

class QObject;

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

          virtual void set_property ( const QObject* sender
                                    , const ::we::type::property::key_type&
                                    , const ::we::type::property::value_type&
                                    ) const;

          void connect_to_change_mgr ( const QObject* object
                                     , const char* signal
                                     , const char* slot
                                     , const char* arguments
                                     ) const;

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
