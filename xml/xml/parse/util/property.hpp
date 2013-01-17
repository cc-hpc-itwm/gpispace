// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_PROPERTY_HPP
#define _XML_PARSE_UTIL_PROPERTY_HPP

#include <xml/parse/error.hpp>
#include <xml/parse/state.hpp>
#include <xml/parse/warning.hpp>

#include <we/type/property.hpp>

#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      namespace property
      {
        namespace property = we::type::property;

        inline void set ( const state::type & state
                        , property::type & prop
                        , const property::path_type & path
                        , const property::value_type & value
                        )
        {
          const boost::optional<property::mapped_type> old
            (prop.set (path, value));

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

        inline void set_state ( state::type & state
                              , property::type & prop
                              , const property::path_type & path
                              , const property::value_type & value
                              = property::value_type()
                              )
        {
          state.interpret_property (path, value);

          ::xml::parse::util::property::set (state, prop, path, value);
        }

        inline void join ( const state::type & state
                         , property::type & x
                         , const property::type & y
                         )
        {
          property::traverse::stack_type stack (property::traverse::dfs (y));

          while (!stack.empty())
            {
              const property::traverse::pair_type & elem (stack.top());

              ::xml::parse::util::property::set
                  (state, x, elem.first, elem.second);

              stack.pop();
            }
        }
      }
    }
  }
}

#endif
