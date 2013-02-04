// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_TRANSITION_HPP
#define _FHG_PNETE_DATA_HANDLE_TRANSITION_HPP 1

#include <pnete/data/handle/transition.fwd.hpp>

#include <pnete/data/handle/function.fwd.hpp>
#include <pnete/data/handle/meta_base.hpp>
#include <pnete/data/handle/net.fwd.hpp>

#include <xml/parse/id/types.hpp>
#include <xml/parse/type/transition.fwd.hpp>

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
        typedef meta_base < ::xml::parse::id::ref::transition
                          , ::xml::parse::type::transition_type
                          > transition_meta_base;
        class transition : public transition_meta_base
        {
        public:
          transition (const transition_meta_base::id_type&, internal_type*);

          void remove (const QObject* sender) const;

          void set_name (const QObject* sender, const QString& type) const;

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

          function function() const;

          bool parent_is (const net&) const;
          net parent() const;

          using transition_meta_base::operator==;
        };
      }
    }
  }
}

#endif
