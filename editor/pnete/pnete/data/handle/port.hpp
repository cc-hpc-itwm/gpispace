// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_PORT_HPP
#define _FHG_PNETE_DATA_HANDLE_PORT_HPP 1

#include <pnete/data/handle/port.fwd.hpp>

#include <pnete/data/change_manager.fwd.hpp>
#include <pnete/data/handle/base.hpp>
#include <pnete/data/handle/function.hpp>

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
        class port : public base
        {
        private:
          typedef ::xml::parse::type::port_type port_type;

        public:
          port ( const port_type& port
               , const handle::function& function
               , change_manager_t& change_manager
               );

          port_type& operator()() const;

          const handle::function& function() const;

          bool operator== (const port& other) const;

          const ::xml::parse::id::port& id() const;

          virtual void set_property ( const QObject* sender
                                    , const ::we::type::property::key_type&
                                    , const ::we::type::property::value_type&
                                    ) const;

          virtual void move ( const QObject* sender
                            , const QPointF& position
                            ) const;

        private:
          ::xml::parse::id::port _port_id;
          handle::function _function;
        };
      }
    }
  }
}

#endif
