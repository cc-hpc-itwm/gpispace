// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_PLACE_HPP
#define _FHG_PNETE_DATA_HANDLE_PLACE_HPP 1

#include <pnete/data/handle/place.fwd.hpp>

#include <pnete/data/change_manager.fwd.hpp>
#include <pnete/data/handle/meta_base.hpp>
#include <pnete/data/handle/net.fwd.hpp>

#include <xml/parse/id/types.hpp>
#include <xml/parse/type/place.fwd.hpp>

class QObject;

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
          place ( const place_meta_base::id_type& id
                , change_manager_t& change_manager
                );

          void remove (const QObject* sender) const;

          virtual void set_property ( const QObject* sender
                                    , const ::we::type::property::key_type&
                                    , const ::we::type::property::value_type&
                                    ) const;

          virtual void move ( const QObject* sender
                            , const QPointF& position
                            ) const;

          virtual void no_undo_move ( const QObject* sender
                                    , const QPointF& position
                                    ) const;

          net parent() const;

          using place_meta_base::operator==;
        };
      }
    }
  }
}

#endif
