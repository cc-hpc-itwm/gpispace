// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_PORT_HPP
#define _FHG_PNETE_DATA_HANDLE_PORT_HPP 1

#include <pnete/data/handle/port.fwd.hpp>

#include <pnete/data/handle/function.fwd.hpp>
#include <pnete/data/handle/meta_base.hpp>
#include <pnete/data/handle/place.fwd.hpp>

#include <we/type/port.hpp>
#include <we/type/property.hpp>

#include <xml/parse/id/types.hpp>
#include <xml/parse/type/port.fwd.hpp>

class QObject;
class QString;

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        typedef meta_base < ::xml::parse::id::ref::port
                          , ::xml::parse::type::port_type
                          > port_meta_base;
        class port : public port_meta_base
        {
        public:
          port (const port_meta_base::id_type&, internal_type*);

          void remove() const;

          void set_name (const QString&) const;
          void set_type (const QString&) const;

          void remove_place_association() const;

          bool is_input() const;
          bool is_output() const;
          bool is_tunnel() const;
          bool direction_is (const we::type::PortDirection&) const;

          bool is_connectable (const port&) const;

          bool is_connected() const;
          place connected_place() const;

          bool can_rename_to (const QString&) const;

          virtual void set_property ( const ::we::type::property::path_type&
                                    , const ::we::type::property::value_type&
                                    ) const override;

          virtual void move (const QPointF& position, const bool outer) const override;
          virtual void no_undo_move (const QPointF& position) const override;

          bool parent_is (const function&) const;
          function parent() const;

          using port_meta_base::operator==;
        };
      }
    }
  }
}

#endif
