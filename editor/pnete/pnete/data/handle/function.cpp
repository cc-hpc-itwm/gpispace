// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/function.hpp>

#include <pnete/data/change_manager.hpp>

#include <fhg/util/backtracing_exception.hpp>

#include <xml/parse/type/transition.hpp>
#include <xml/parse/type/function.hpp>

#include <boost/optional.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        function::function ( const function_type& function
                           , const handle::transition& transition
                           , change_manager_t& change_manager
                           )
          : base (change_manager)
          , _function_id (function.id())
          , _transition (transition)
        { }

        function::function_type& function::operator()() const
        {
          const boost::optional<function_type&> function
            (transition()().function_by_id_ref (_function_id));
          if (!function)
          {
            throw fhg::util::backtracing_exception
              ("INVALID HANDLE: function id not found");
          }
          return *function;
        }

        const handle::transition& function::transition() const
        {
          return _transition;
        }

        bool function::operator== (const function& other) const
        {
          return _function_id == other._function_id
            && _transition == other._transition;
        }

        const ::xml::parse::id::function& function::id() const
        {
          return _function_id;
        }

        void function::set_property ( const QObject* sender
                                    , const ::we::type::property::key_type& key
                                    , const ::we::type::property::value_type& val
                                    ) const
        {
          change_manager().set_property (sender, *this, key, val);
        }
      }
    }
  }
}
