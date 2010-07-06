// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_WARNING_HPP
#define _XML_PARSE_WARNING_HPP

#include <stdexcept>
#include <string>
#include <sstream>

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

      // ******************************************************************* //

      class overwrite_function_name : public generic
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


      // ******************************************************************* //

      template<typename T>
      class struct_shadowed : public generic
      {
      private:
        std::string nice (const T & early, const T & late) const
        {
          std::ostringstream s;

          s << "struct with name " << late.name
            << " in " << late.path
            << " shadows definition from " << early.path
            ;

          return s.str();
        }

      public:
        struct_shadowed (const T & early, const T & late)
          : generic (nice (early, late))
        {}
      };
    }
  }
}

#endif
