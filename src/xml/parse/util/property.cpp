#include <xml/parse/util/property.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/warning.hpp>

#include <we/type/value/positions.hpp>

#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      namespace property
      {
        namespace
        {
          void set ( const state::type& state
                   , we::type::property::type& prop
                   , const we::type::property::path_type& path
                   , const we::type::property::value_type& value
                   )
          {
            const boost::optional<pnet::type::value::value_type> old
              (prop.get (path));
            prop.set (path, value);

            if (old)
            {
              state.warn
                ( warning::property_overwritten ( path
                                                , *old
                                                , value
                                                , state.file_in_progress()
                                                )
                );
            }
          }
        }

        void set_state ( state::type& state
                       , we::type::property::type& prop
                       , const we::type::property::path_type& path
                       , const we::type::property::value_type& value
                       )
        {
          state.interpret_property (path, value);

          ::xml::parse::util::property::set (state, prop, path, value);
        }

        void join ( const state::type& state
                  , we::type::property::type& x
                  , const we::type::property::type& y
                  )
        {
          typedef std::pair< std::list<std::string>
                           , pnet::type::value::value_type
                           > path_and_value_type;

          for ( path_and_value_type const& path_and_value
              : pnet::type::value::positions (y.value())
              )
          {
            ::xml::parse::util::property::set
              ( state
              , x
              , path_and_value.first
              , path_and_value.second
              );
          }
        }
      }
    }
  }
}
