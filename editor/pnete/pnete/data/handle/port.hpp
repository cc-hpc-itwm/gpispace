// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_PORT_HPP
#define _FHG_PNETE_DATA_HANDLE_PORT_HPP 1

#include <pnete/data/handle/port.fwd.hpp>

#include <pnete/data/handle/meta_base.hpp>
#include <pnete/data/handle/function.fwd.hpp>

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

          void remove (const QObject* sender) const;

          void set_name (const QObject* sender, const QString& type) const;
          void set_type (const QObject* sender, const QString& type) const;

          void remove_place_association (const QObject* sender) const;

          bool is_input() const;
          bool is_output() const;
          bool is_tunnel() const;

          bool is_connectable (const port&) const;

          bool is_connected() const;

          bool can_rename_to (const QString&) const;

          virtual void set_property ( const QObject* sender
                                    , const ::we::type::property::key_type&
                                    , const ::we::type::property::value_type&
                                    ) const;

          virtual void move ( const QObject* sender
                            , const QPointF& position
                            , const bool outer
                            ) const;

          virtual void no_undo_move ( const QObject* sender
                                    , const QPointF& position
                                    ) const;

          bool parent_is (const function&) const;
          function parent() const;

          using port_meta_base::operator==;
        };
      }
    }
  }
}

#endif
