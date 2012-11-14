// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_PORT_HPP
#define _FHG_PNETE_DATA_HANDLE_PORT_HPP 1

#include <pnete/data/handle/port.fwd.hpp>

#include <pnete/data/change_manager.fwd.hpp>
#include <pnete/data/handle/meta_base.hpp>
#include <pnete/data/handle/function.fwd.hpp>

#include <xml/parse/id/types.hpp>
#include <xml/parse/type/port.fwd.hpp>

class QObject;

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        class port : public meta_base < ::xml::parse::id::ref::port
                                      , ::xml::parse::type::port_type
                                      >
        {
        public:
          port ( const meta_base::id_type& id
               , change_manager_t& change_manager
               );

          virtual void set_property ( const QObject* sender
                                    , const ::we::type::property::key_type&
                                    , const ::we::type::property::value_type&
                                    ) const;

          virtual void move ( const QObject* sender
                            , const QPointF& position
                            ) const;

          function parent() const;
        };
      }
    }
  }
}

#endif
