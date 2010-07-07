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

      class wrong_node : public generic
      {
      public:
        wrong_node ( const rapidxml::node_type & want
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

      class missing_node : public generic
      {
      public:
        missing_node (const rapidxml::node_type & want)
          : generic ( "missing node"
                    , "expected node of type "
                    + util::quote(util::show_node_type (want))
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

      class place_type_unknown : public generic
      {
      private:
        std::string nice ( const std::string & place
                         , const std::string & type
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << "unknown type " << type
            << " given to place " << place
            << " in " << path
            ;

          return s.str();
        }
      public:
        place_type_unknown ( const std::string & place
                           , const std::string & type
                           , const boost::filesystem::path & path
                           )
          : generic (nice (place, type, path))
        {}
      };

      // ******************************************************************* //

      class parse_incomplete : public generic
      {
      private:
        std::string nice ( const std::string & place
                         , const std::string & field
                         , const literal::type_name_t & sig
                         , const literal::type_name_t & val
                         , const std::string & rest
                         , const boost::filesystem::path & path
                         )
        {
          std::ostringstream s;
          
          s << "when try to read value for place " << place
            << " from " << path
            << " when reading for field " << field
            << " a value of type " << sig
            << " from " << val
            << " there is the rest " << rest
            ;

          return s.str();
        }

      public:
        parse_incomplete ( const std::string & place
                         , const std::string & field
                         , const literal::type_name_t & sig
                         , const literal::type_name_t & val
                         , const std::string & rest
                         , const boost::filesystem::path & path
                         )
          : generic (nice (place, field, sig, val, rest, path))
        {}
      };

      // ******************************************************************* //

      class parse_type_mismatch : public generic
      {
      private:
        std::string nice  ( const std::string & place
                          , const std::string & field
                          , const literal::type_name_t & sig
                          , const signature::structured_t & val
                          , const boost::filesystem::path & path
                          )
        {
          std::ostringstream s;

          s << "type mismatch when try to read a value for place " << place
            << " from " << path
            << " when reading field " << field
            << " expecting literal of type " << sig
            << " got structured value " << val
            ;

          return s.str();
        }

        std::string nice  ( const std::string & place
                          , const std::string & field
                          , const signature::structured_t & sig
                          , const literal::type_name_t & val
                          , const boost::filesystem::path & path
                          )
        {
          std::ostringstream s;

          s << "type mismatch when try to read a value for place " << place
            << " from " << path
            << " when reading field " << field
            << " expecting structured type " << sig
            << " got literal value " << val
            ;

          return s.str();
        }

      public:
        parse_type_mismatch ( const std::string & place
                            , const std::string & field
                            , const literal::type_name_t & sig
                            , const signature::structured_t & val
                            , const boost::filesystem::path & path
                            )
          : generic (nice (place, field, sig, val, path))
        {}

        parse_type_mismatch ( const std::string & place
                            , const std::string & field
                            , const signature::structured_t & sig
                            , const literal::type_name_t & val
                            , const boost::filesystem::path & path
                            )
          : generic (nice (place, field, sig, val, path))
        {}
      };

      // ******************************************************************* //

      class parse_lift : public generic
      {
      private:
        std::string nice ( const std::string & place
                         , const std::string & field
                         , const boost::filesystem::path & path
                         , const std::string & msg
                         )
        {
          std::ostringstream s;

          s << "when reading a value for place " << place;

          if (field != "")
            {
              s << " for field " << field;
            }

          s << " from "<< path
            << ": " << msg
            ;

          return s.str();
        }

      public:
        parse_lift ( const std::string & place
                   , const std::string & field
                   , const boost::filesystem::path & path
                   , const std::string & msg
                   )
          : generic (nice (place, field, path, msg))
        {}
      };

      // ******************************************************************* //

      template<typename T>
      class forbidden_shadowing : public generic
      {
      private:
        std::string nice ( const T & early
                         , const T & late
                         , const std::string & port_name
                         ) const
        {
          std::ostringstream s;

          s << "struct with name " << late.name
            << " in " << late.path
            << " shadows definition from " << early.path
            << " but this is forbidden, since it is a type used for port "
            << port_name
            ;

          return s.str();
        }

      public:
        forbidden_shadowing ( const T & early
                            , const T & late
                            , const std::string & port_name
                            )
          : generic (nice (early, late, port_name))
        {}
      };

      // ******************************************************************* //

      class duplicate_place : public generic
      {
      private:
        std::string nice ( const std::string & place
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << "duplicate place " << place << " in " << path;

          return s.str();
        }

      public:
        duplicate_place ( const std::string & place
                        , const boost::filesystem::path & path
                        )
          : generic (nice (place, path))
        {}
      };

      // ******************************************************************* //

      class duplicate_port : public generic
      {
      private:
        std::string nice ( const std::string & direction
                         , const std::string & port
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << "duplicate " << direction << "-port " << port
            << " in " << path
            ;

          return s.str();
        }

      public:
        duplicate_port ( const std::string & direction
                       , const std::string & port
                       , const boost::filesystem::path & path
                       )
          : generic (nice (direction, port, path))
        {}
      };

      // ******************************************************************* //

      template<typename T>
      class duplicate_transition : public generic
      {
      private:
        std::string nice (const T & t, const T & old) const
        {
          std::ostringstream s;

          s << "duplicate transition " << t.name << " in " << t.path
            << " first defintion was in " << old.path
            ;

          return s.str();
        }
      public:
        duplicate_transition (const T & t, const T & old)
          : generic (nice (t, old))
        {}
      };

      // ******************************************************************* //

      template<typename T>
      class duplicate_function : public generic
      {
      private:
        std::string nice (const T & t, const T & old) const
        {
          std::ostringstream s;

          s << "duplicate function " << t.name << " in " << t.path
            << " first defintion was in " << old.path
            ;

          return s.str();
        }
      public:
        duplicate_function (const T & t, const T & old)
          : generic (nice (t, old))
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
