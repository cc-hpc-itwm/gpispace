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

          void remove (const QObject* sender) const;

          void set_name (const QObject* sender, const QString& type) const;
          void set_type (const QObject* sender, const QString& type) const;

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

          net parent() const;

          bool is_implicit() const;
          void make_explicit (const QObject*) const;

          using place_meta_base::operator==;
        };
      }
    }
  }
}

#endif
