// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/transition.hpp>

#include <pnete/data/handle/net.hpp>

#include <xml/parse/type/net.hpp>
#include <xml/parse/type/transition.hpp>

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
        transition::transition ( const transition_type& transition
                               , const handle::net& net
                               )
          : _transition_id (transition.id())
          , _net (net)
        { }

        transition::transition_type transition::operator()() const
        {
          const boost::optional<transition_type> transition
            (net()().transition_by_id (_transition_id));
          if (!transition)
          {
            throw fhg::util::backtracing_exception
              ("INVALID HANDLE: transition id not found");
          }
          return *transition;
        }

        const handle::net& transition::net() const
        {
          return _net;
        }

        bool transition::operator== (const transition& other) const
        {
          return _transition_id == other._transition_id && _net == other._net;
        }

        const ::xml::parse::id::transition& transition::id() const
        {
          return _transition_id;
        }
      }
    }
  }
}

