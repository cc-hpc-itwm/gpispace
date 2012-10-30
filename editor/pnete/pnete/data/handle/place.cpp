// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/place.hpp>

#include <pnete/data/change_manager.hpp>
#include <pnete/data/internal.hpp>

#include <fhg/util/backtracing_exception.hpp>

#include <xml/parse/type/net.hpp>
#include <xml/parse/type/place.hpp>

#include <boost/optional.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        place::place ( const place_type& place
                     , const handle::net& net
                     , change_manager_t& change_manager
                     )
          : _place_id (place.id())
          , _net (net)
          , _change_manager (change_manager)
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

        void place::remove (const QObject* sender) const
        {
          change_manager().delete_place (sender, *this);
        }

        change_manager_t& place::change_manager() const
        {
          return _change_manager;
        }
      }
    }
  }
}
