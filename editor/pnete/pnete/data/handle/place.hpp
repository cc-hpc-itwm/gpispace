// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_PLACE_HPP
#define _FHG_PNETE_DATA_HANDLE_PLACE_HPP 1

#include <pnete/data/handle/net.hpp>

#include <xml/parse/type/place.fwd.hpp>
#include <xml/parse/util/id_type.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        class place
        {
        private:
          typedef ::xml::parse::type::place_type place_type;

        public:
          place (const place_type& place, const handle::net& net);

          place_type operator()() const;

          const handle::net& net() const;

          bool operator== (const place& other) const;

        private:
          ::fhg::xml::parse::util::id_type _place_id;
          handle::net _net;
        };
      }
    }
  }
}

#endif
