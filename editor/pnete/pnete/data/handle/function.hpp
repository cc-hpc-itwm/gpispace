// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_FUNCTION_HPP
#define _FHG_PNETE_DATA_HANDLE_FUNCTION_HPP 1

#include <pnete/data/handle/function.fwd.hpp>

#include <pnete/data/change_manager.fwd.hpp>
#include <pnete/data/handle/base.hpp>
#include <pnete/data/handle/transition.hpp>

#include <xml/parse/id/types.hpp>
#include <xml/parse/type/function.fwd.hpp>

#include <boost/optional.hpp>

class QObject;

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        class function : public base
        {
        private:
          typedef ::xml::parse::type::function_type function_type;

        public:
          function ( const function_type& function
                   , boost::optional<const handle::transition&> transition
                   , change_manager_t& change_manager
                   );

          function_type& operator()() const;

          const handle::transition& transition() const;

          bool operator== (const function& other) const;

          const ::xml::parse::id::function& id() const;

          virtual void set_property ( const QObject* sender
                                    , const ::we::type::property::key_type&
                                    , const ::we::type::property::value_type&
                                    ) const;

        private:
          ::xml::parse::id::function _function_id;
          boost::optional<handle::transition> _transition;
          ::xml::parse::type::function_type& _BAD_BAD_FUNCTION_REFERENCE;
        };
      }
    }
  }
}

#endif
