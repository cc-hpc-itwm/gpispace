// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_ERROR_HPP
#define _XML_PARSE_ERROR_HPP

#include <stdexcept>
#include <string>
#include <sstream>

#include <boost/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    namespace error
    {
      // ******************************************************************* //

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

      // ******************************************************************* //

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

      // ******************************************************************* //

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

      // ******************************************************************* //

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

      // ******************************************************************* //

      class no_elements_given : public generic
      {
      public:
        no_elements_given (const std::string & pre)
          : generic ("no elements given at all!?", pre)
        {}
      };

      // ******************************************************************* //

      class more_than_one_definition : public generic
      {
      public:
        more_than_one_definition (const std::string & pre)
          : generic ("more than one definition in one file", pre)
        {}
      };

      // ******************************************************************* //

      class top_level_anonymous_function : public generic
      {
      public:
        top_level_anonymous_function ( const std::string & file
                                     , const std::string & pre
                                     )
          : generic ( "try to include top level anonymous function from " 
                    + file
                    , pre
                    )
        {}
      };

      // ******************************************************************* //

      class file_not_found : public generic
      {
      public:
        file_not_found ( const std::string & pre
                       , const std::string & file
                       )
          : generic (pre, "file not found: " + file)
        {}
      };

      // ******************************************************************* //

      template<typename IT>
      class include_loop : public generic
      {
      private:
        std::string nice (IT pos, const IT & end) const
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

      // ******************************************************************* //

      class cannot_resolve : public generic
      {
      private:
        std::string nice ( const std::string & field
                         , const std::string & type
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << "cannot resolve " << field << " :: " << type
            << ", defined in " << path;

          return s.str();
        }
        
      public:
        cannot_resolve ( const std::string & field
                       , const std::string & type
                       , const boost::filesystem::path & path
                       )
          : generic (nice (field, type, path))
        {}
      };

      // ******************************************************************* //

      template<typename T>
      class struct_redefined : public generic
      {
      private:
        std::string nice (const T & early, const T & late) const
        {
          std::ostringstream s;

          s << "redefinition of struct with name " << late.name
            << " in " << late.path
            << ", first definition was in " << early.path
            ;

          return s.str();
        }

      public:
        struct_redefined (const T & early, const T & late)
          : generic (nice (early, late))
        {}
      };

      // ******************************************************************* //

      class strange : public generic
      {
      public:
        strange (const std::string & msg) : generic ("STRANGE", msg) {}
      };
    }
  }
}

#endif
