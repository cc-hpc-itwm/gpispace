// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_ERROR_HPP
#define _XML_PARSE_ERROR_HPP

#include <stdexcept>
#include <string>

namespace xml
{
  namespace parse
  {
    namespace error
    {
      class generic : public std::runtime_error
      {
      public:
        generic (const std::string & msg)
          : std::runtime_error ("ERROR: " + msg)
        {}

        generic (const std::string & msg, const std::string & pre)
          : std::runtime_error ("ERROR: " + pre + ": " + msg)
        {}
      };

      class unexpected_element : public generic
      {
      public:
        unexpected_element ( const std::string & name
                           , const std::string & pre
                           )
          : generic
            (pre + ": unexpected element with name " + util::quote(name))
        {}
      };

      class missing_attr : public generic
      {
      public:
        missing_attr ( const std::string & pre
                     , const std::string & attr
                     )
          : generic
            (pre + ": missing attribute " + util::quote(attr))
        {}
      };

      class no_elements_given : public generic
      {
      public:
        no_elements_given (const std::string & pre)
          : generic ("no elements given at all!?", pre)
        {}
      };

      class more_than_one_definition : public generic
      {
      public:
        more_than_one_definition (const std::string & pre)
          : generic ("more than one definition in one file", pre)
        {}
      };

      class top_level_anonymous_function : public generic
      {
      public:
        top_level_anonymous_function ( const std::string & file
                                     , const std::string & pre
                                     )
          : generic ( "try to include top level anonymous function from file" 
                    + file
                    , pre
                    )
        {}
      };
    }
  }
}

#endif
