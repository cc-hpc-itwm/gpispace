// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_TRANSITION_HPP
#define _FHG_PNETE_DATA_HANDLE_TRANSITION_HPP 1

#include <pnete/data/handle/net.hpp>

#include <xml/parse/type/transition.fwd.hpp>

#include <xml/parse/util/id_type.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        class transition
        {
        private:
          typedef ::xml::parse::type::transition_type transition_type;

        public:
          transition ( const transition_type& transition
                     , const handle::net& net
                     );

          transition_type operator()() const;

          const handle::net& net() const;

          bool operator== (const transition& other) const;

        private:
          ::xml::parse::id::transition _transition_id;
          handle::net _net;
        };
      }
    }
  }
}

#endif
