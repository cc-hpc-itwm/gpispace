// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_PLACE_HPP
#define _FHG_PNETE_DATA_HANDLE_PLACE_HPP 1

#include <pnete/data/handle/place.fwd.hpp>

#include <pnete/data/handle/meta_base.hpp>
#include <pnete/data/handle/net.fwd.hpp>

#include <xml/parse/id/types.hpp>
#include <xml/parse/type/place.fwd.hpp>

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
        typedef meta_base < ::xml::parse::id::ref::place
                          , ::xml::parse::type::place_type
                          > place_meta_base;
        class place : public place_meta_base
        {
        public:
          place (const place_meta_base::id_type&, internal_type*);

          void remove() const;

          void set_name (const QString&) const;
          void set_type (const QString&) const;

          bool can_rename_to (const QString&) const;

          virtual void set_property ( const ::we::type::property::key_type&
                                    , const ::we::type::property::value_type&
                                    ) const;

          virtual void move
            (const QPointF& position, const bool outer) const;

          virtual void no_undo_move (const QPointF& position) const;

          bool is_implicit() const;
          void make_explicit() const;

          bool is_virtual() const;
          void make_virtual() const;
          void make_real() const;

          bool parent_is (const net&) const;
          net parent() const;

          using place_meta_base::operator==;
        };
      }
    }
  }
}

#endif
