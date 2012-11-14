// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/place.hpp>

#include <pnete/data/change_manager.hpp>
#include <pnete/data/handle/net.hpp>

#include <xml/parse/type/net.hpp>
#include <xml/parse/type/place.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        place::place ( const ::xml::parse::id::ref::place& id
                     , change_manager_t& change_manager
                     )
          : base (change_manager)
          , _id (id)
        { }


        const ::xml::parse::type::place_type& place::get() const
        {
          return _id.get();
        }
        ::xml::parse::type::place_type& place::get_ref() const
        {
          return _id.get_ref();
        }

        bool place::operator== (const place& other) const
        {
          return _id == other._id;
        }

        void place::remove (const QObject* sender) const
        {
          change_manager().delete_place (sender, *this);
        }

        const ::xml::parse::id::ref::place& place::id() const
        {
          return _id;
        }

        net place::parent() const
        {
          return net (get().parent()->make_reference_id(), change_manager());
        }
      }
    }
  }
}
