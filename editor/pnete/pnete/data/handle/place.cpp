// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/place.hpp>

#include <pnete/data/handle/net.hpp>

//! \todo Only include place.hpp and net.hpp?
#include <xml/parse/types.hpp>

#include <fhg/util/backtracing_exception.hpp>

#include <boost/optional.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        place::place (const place_type& place, const handle::net& net)
          : _place_id (place.id())
          , _net (net)
        { }

        place::place_type place::operator()() const
        {
          const boost::optional<place_type> place
            (net()().place_by_id (_place_id));
          if (!place)
          {
            throw fhg::util::backtracing_exception
              ("INVALID HANDLE: place id not found");
          }
          return *place;
        }

        const handle::net& place::net() const
        {
          return _net;
        }

        bool place::operator== (const place& other) const
        {
          return _place_id == other._place_id && _net == other._net;
        }

        const ::xml::parse::id::place& place::id() const
        {
          return _place_id;
        }
      }
    }
  }
}

