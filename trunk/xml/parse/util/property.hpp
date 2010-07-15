// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_PROPERTY_HPP
#define _XML_PARSE_UTIL_PROPERTY_HPP

#include <xml/parse/error.hpp>
#include <xml/parse/state.hpp>
#include <xml/parse/warning.hpp>

#include <xml/parse/util/maybe.hpp>

#include <we/type/property.hpp>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      namespace property
      {
        inline void set ( const state::type & state
                        , we::type::property::type & prop
                        , const we::type::property::path_type & path
                        , const we::type::property::value_type & value
                        )
        {
          const maybe<we::type::property::value_type>
            old (prop.get_maybe_val (path));

          const bool overwritten (prop.set (path, value));

          if (overwritten)
            {
              if (old.isNothing())
                {
                  THROW_STRANGE
                    ("property_set: old.isNothing() and overwritten == true");
                }

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
    }
  }
}

#endif
