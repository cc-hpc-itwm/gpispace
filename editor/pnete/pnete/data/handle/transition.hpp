// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_TRANSITION_HPP
#define _FHG_PNETE_DATA_HANDLE_TRANSITION_HPP 1

#include <pnete/data/handle/transition.fwd.hpp>

#include <pnete/data/change_manager.fwd.hpp>
#include <pnete/data/handle/base.hpp>
#include <pnete/data/handle/net.fwd.hpp>

#include <xml/parse/id/types.hpp>
#include <xml/parse/type/transition.fwd.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        class transition : public base
        {
        public:
          transition ( const ::xml::parse::id::ref::transition& id
                     , change_manager_t& change_manager
                     );

          const ::xml::parse::type::transition_type& get() const;
          ::xml::parse::type::transition_type& get_ref() const;

          bool operator== (const transition& other) const;

          const ::xml::parse::id::ref::transition& id() const;

          virtual void set_property ( const QObject* sender
                                    , const ::we::type::property::key_type&
                                    , const ::we::type::property::value_type&
                                    ) const;

          net parent() const;

        private:
          ::xml::parse::id::ref::transition _id;
        };
      }
    }
  }
}

#endif
