// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_EXCEPTION_HPP
#define _XML_PARSE_EXCEPTION_HPP

#include <stdexcept>
#include <string>
#include <sstream>

namespace xml
{
  namespace parse
  {
    namespace exception
    {
      class generic : public std::runtime_error
      {
      public:
        generic (const std::string & pre, const std::string & msg)
          : std::runtime_error (pre + ": " + msg)
        {}
      };

      class unexpected_element : public generic
      {
      public:
        unexpected_element ( const std::string & pre
                           , const std::string & name
                           )
          : generic (pre, "unexpected element with name " + util::quote(name))
        {}
      };

      class node_type : public generic
      {
      public:
        node_type (const rapidxml::node_type & want)
          : generic ( "missing node"
                    , "expected node of type "
                    + util::quote(util::show_node_type (want))
                    )
        {}
        node_type ( const rapidxml::node_type & want
                  , const rapidxml::node_type & got
                  )
          : generic ("wrong node type"
                    , "expexted node of type "
                    + util::quote(util::show_node_type (want))
                    + ": got node of type "
                    + util::quote(util::show_node_type (got))
                    )
        {}
      };

      class missing_attr : public generic
      {
      public:
        missing_attr ( const std::string & pre
                     , const std::string & attr
                     )
          : generic (pre, "missing attribute " + util::quote(attr))
        {}
      };

      class file_not_found : public generic
      {
      public:
        file_not_found ( const std::string & pre
                       , const std::string & file
                       )
          : generic (pre, "file not found: " + file)
        {}
      };

      template<typename IT>
      class include_loop : public generic
      {
      private:
        std::string nice (IT pos, const IT & end)
        {
          std::ostringstream ss;

          IT loop (pos);

          for  (; pos != end; ++pos)
            {
              ss << *pos << " -> ";
            }

          ss << *loop;

          return ss.str();
        }
        
      public:
        include_loop ( const std::string & pre
                     , IT pos, const IT & end
                     )
          : generic (pre, "include loop: " + nice (pos, end))
        {}
      };
    }
  }
}

#endif
