// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_EXCEPTION_HPP
#define _XML_PARSE_EXCEPTION_HPP

#include <stdexcept>
#include <string>

namespace xml
{
  namespace parse
  {
    namespace exception
    {
      class unexpected_element : public std::runtime_error
      {
      public:
        unexpected_element ( const std::string & pre
                           , const std::string & name
                           )
          : std::runtime_error 
            (pre + ": unexpected element with name " + util::quote(name))
        {}
      };

      class node_type : public std::runtime_error
      {
      public:
        node_type (const rapidxml::node_type & want)
          : std::runtime_error ("missing node: expected node of type "
                               + util::quote(util::show_node_type (want))
                               )
        {}
        node_type ( const rapidxml::node_type & want
                  , const rapidxml::node_type & got
                  )
          : std::runtime_error ("wrong node type: expexted node of type "
                               + util::quote(util::show_node_type (want))
                               + ": got node of type "
                               + util::quote(util::show_node_type (got))
                               )
        {}
      };

      class missing_attr : public std::runtime_error
      {
      public:
        missing_attr ( const std::string & pre
                     , const std::string & attr
                     )
          : std::runtime_error
            (pre + ": missing attribute " + util::quote(attr))
        {}
      };

      class error : public std::runtime_error
      {
      public:
        error (const std::string & pre, const std::string & msg)
          : std::runtime_error (pre + ": " + msg)
        {}
      };

      class strange : public std::runtime_error
      {
      public:
        strange (const std::string & msg)
          : std::runtime_error ("STRANGE! " + msg)
        {}
      };
    }
  }
}

#endif
