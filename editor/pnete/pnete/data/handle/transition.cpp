// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/transition.hpp>

#include <pnete/data/handle/net.hpp>

#include <xml/parse/type/net.hpp>
#include <xml/parse/type/transition.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        transition::transition ( const ::xml::parse::id::ref::transition& id
                               , change_manager_t& change_manager
                               )
          : base (change_manager)
          , _id (id)
        { }

        const ::xml::parse::type::transition_type& transition::get() const
        {
          return _id.get();
        }
        ::xml::parse::type::transition_type& transition::get_ref() const
        {
          return _id.get_ref();
        }

        bool transition::operator== (const transition& other) const
        {
          return _id == other._id;
        }

        const ::xml::parse::id::ref::transition& transition::id() const
        {
          return _id;
        }

        net transition::parent() const
        {
          return net (get().parent()->make_reference_id(), change_manager());
        }
      }
    }
  }
}
