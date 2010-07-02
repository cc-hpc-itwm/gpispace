// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_WARNING_HPP
#define _XML_PARSE_WARNING_HPP

#include <stdexcept>
#include <string>

namespace xml
{
  namespace parse
  {
    namespace warning
    {
      class generic : public std::runtime_error
      {
      public:
        generic (const std::string & msg)
          : std::runtime_error ("WARNING: " + msg)
        {}
      };

      class overwrite_function_name : generic
      {
      public:
        overwrite_function_name ( const std::string & old_name
                                , const std::string & new_name
                                )
          : generic ( "old function name " + old_name
                    + " overwritten by new name " + new_name
                    )
        {}
      };
    }
  }
}

#endif
