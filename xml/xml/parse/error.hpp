// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _XML_PARSE_ERROR_HPP
#define _XML_PARSE_ERROR_HPP

#include <string>
#include <sstream>

#include <xml/parse/id/types.hpp>

#include <we/type/literal.hpp>
#include <we/type/signature.hpp>
#include <we/type/port.hpp>
#include <we/type/property.hpp>

#include <fhg/util/join.hpp>
#include <fhg/util/backtracing_exception.hpp>

#include <xml/parse/util/show_node_type.hpp> // WORK HERE: for quote only

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    namespace error
    {
      // ******************************************************************* //

#ifndef NO_BACKTRACE_ON_PARSE_ERROR
#define GENERIC_EXCEPTION_BASE_CLASS fhg::util::backtracing_exception
#else
#define GENERIC_EXCEPTION_BASE_CLASS std::runtime_error
#endif

      class generic : public GENERIC_EXCEPTION_BASE_CLASS
      {
      public:
        generic (const std::string & msg)
          : GENERIC_EXCEPTION_BASE_CLASS ("ERROR: " + msg)
        { }

        generic (const boost::format& bf)
          : GENERIC_EXCEPTION_BASE_CLASS ("ERROR: " + bf.str())
        { }

        generic (const std::string & msg, const std::string & pre)
          : GENERIC_EXCEPTION_BASE_CLASS ("ERROR: " + pre + ": " + msg)
        { }
      };

#undef GENERIC_EXCEPTION_BASE_CLASS

      // ******************************************************************* //

      class wrong_node : public generic
      {
      private:
        std::string nice ( const rapidxml::node_type & want
                         , const rapidxml::node_type & got
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << "expected node of type "
            << util::quote (util::show_node_type (want))
            << ": got node of type "
            << util::quote(util::show_node_type (got))
            << " in " << path
            ;

          return s.str();
        }

        std::string nice ( const rapidxml::node_type & want1
                         , const rapidxml::node_type & want2
                         , const rapidxml::node_type & got
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << "expected node of type "
            << util::quote (util::show_node_type (want1))
            << " or "
            << util::quote (util::show_node_type (want2))
            << ": got node of type "
            << util::quote(util::show_node_type (got))
            << " in " << path
            ;

          return s.str();
        }

      public:
        wrong_node ( const rapidxml::node_type & want
                   , const rapidxml::node_type & got
                   , const boost::filesystem::path & path
                   )
          : generic ( "wrong node", nice (want, got, path))
        {}

        wrong_node ( const rapidxml::node_type & want1
                   , const rapidxml::node_type & want2
                   , const rapidxml::node_type & got
                   , const boost::filesystem::path & path
                     )
          : generic ("wrong node", nice (want1, want2, got, path))
        {}
      };

      // ******************************************************************* //

      class missing_node : public generic
      {
      private:
        std::string nice ( const rapidxml::node_type & want
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << "expected node of type "
            << util::quote (util::show_node_type (want))
            << " in " << path
            ;

          return s.str();
        }

        std::string nice ( const rapidxml::node_type & want1
                         , const rapidxml::node_type & want2
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << "expected node of type "
            << util::quote (util::show_node_type (want1))
            << " or "
            << util::quote (util::show_node_type (want2))
            << " in " << path
            ;

          return s.str();
        }

      public:
        missing_node ( const rapidxml::node_type & want
                     , const boost::filesystem::path & path
                     )
          : generic ( "missing node", nice (want, path))
        {}

        missing_node ( const rapidxml::node_type & want1
                     , const rapidxml::node_type & want2
                     , const boost::filesystem::path & path
                     )
          : generic ("missing node", nice (want1, want2, path))
        {}
      };

      // ******************************************************************* //

     class missing_attr : public generic
      {
      private:
        std::string nice ( const std::string & pre
                         , const std::string & attr
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << pre
            << ": missing attribute "
            << util::quote (attr)
            << " in " << path
            ;

          return s.str();
        }

      public:
        missing_attr ( const std::string & pre
                     , const std::string & attr
                     , const boost::filesystem::path & path
                     )
          : generic (nice (pre, attr, path))
        {}
      };

      // ******************************************************************* //

      class no_elements_given : public generic
      {
      private:
        std::string nice (const boost::filesystem::path & path) const
        {
          std::ostringstream s;

          s << "no elements given at all"
            << " in " << path
            ;

          return s.str();
        }

      public:
        no_elements_given ( const std::string & pre
                          , const boost::filesystem::path & path
                          )
          : generic (nice (path), pre)
        {}
      };

      // ******************************************************************* //

      class more_than_one_definition : public generic
      {
      private:
        std::string nice (const boost::filesystem::path & path) const
        {
          std::ostringstream s;

          s << "more than one definition"
            << " in " << path
            ;

          return s.str();
        }

      public:
        more_than_one_definition ( const std::string & pre
                                 , const boost::filesystem::path & path
                                 )
          : generic (nice (path), pre)
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

      class top_level_anonymous_template : public generic
      {
      public:
        top_level_anonymous_template ( const std::string & file
                                     , const std::string & pre
                                     )
          : generic ( "try to include top level anonymous template from "
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

      class file_already_there : public generic
      {
      private:
        std::string nice (const boost::filesystem::path& file) const
        {
          std::ostringstream ss;

          ss << "file " << file << " already there with a different content";

          return ss.str();
        }

      public:
        explicit file_already_there (const boost::filesystem::path& file)
          : generic (nice (file))
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

          s << "redefinition of struct with name " << late.name()
            << " in " << late.path()
            << ", first definition was in " << early.path()
            ;

          return s.str();
        }

      public:
        struct_redefined (const T & early, const T & late)
          : generic (nice (early, late))
        {}
      };

      class struct_field_redefined : public generic
      {
      public:
        struct_field_redefined ( const std::string& name
                               , const boost::filesystem::path& path
                               )
          : generic ( boost::format ("struct field '%1%' redefined in %2%")
                    % name % path
                    )
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

      class parse_type_mismatch : public generic
      {
      private:
        std::string nice  ( const std::string & place
                          , const std::string & field
                          , const literal::type_name_t & sig
                          , const signature::structured_t & val
                          , const boost::filesystem::path & path
                          ) const
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
                          ) const
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
                         ) const
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

      class eval_context_bind : public generic
      {
      public:
        eval_context_bind ( const std::string & descr
                          , const std::string & msg
                          )
          : generic (descr + ": " + msg)
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

          s << "struct with name " << late.name()
            << " in " << late.path()
            << " shadows definition from " << early.path()
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
      public:
        duplicate_port ( const id::ref::port& port
                       , const id::ref::port& old_port
                       , const boost::filesystem::path & path
                       );
        virtual ~duplicate_port() throw() { }

      private:
        const id::ref::port _port;
        const id::ref::port _old_port;
        const boost::filesystem::path _path;
      };

      // ******************************************************************* //

      class duplicate_connect : public generic
      {
      public:
        duplicate_connect ( const id::ref::connect& connection
                          , const id::ref::connect& old_connection
                          , const id::ref::transition& transition
                          , const boost::filesystem::path& path
                          );
        virtual ~duplicate_connect() throw() { }

      private:
        const id::ref::connect _connection;
        const id::ref::connect _old_connection;
        const id::ref::transition _transition;
        const boost::filesystem::path _path;
      };

      // ******************************************************************* //

      class duplicate_place_map : public generic
      {
      public:
        duplicate_place_map ( const id::ref::place_map& place_map
                            , const id::ref::place_map& old_place_map
                            , const id::ref::transition& transition
                            , const boost::filesystem::path & path
                            );
        virtual ~duplicate_place_map() throw() { }

      private:
        const id::ref::place_map _place_map;
        const id::ref::place_map _old_place_map;
        const id::ref::transition _transition;
        const boost::filesystem::path _path;
      };

      // ******************************************************************* //

      class duplicate_specialize : public generic
      {
      private:
        std::string nice ( const std::string & name
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << "duplicate specialize " << name << " in " << path;

          return s.str();
        }

      public:
        duplicate_specialize ( const std::string & name
                             , const boost::filesystem::path & path
                             )
          : generic (nice (name, path))
        {}
      };

      // ******************************************************************* //

      class duplicate_transition : public generic
      {
      public:
        duplicate_transition ( const id::ref::transition& transition
                             , const id::ref::transition& old_transition
                             );
        ~duplicate_transition() throw() { }

      private:
        id::ref::transition _transition;
        id::ref::transition _old_transition;
      };

      // ******************************************************************* //

      class duplicate_function : public generic
      {
      public:
        duplicate_function ( const id::ref::function& function
                           , const id::ref::function& old_function
                           );
        ~duplicate_function() throw() { }

      private:
        id::ref::function _function;
        id::ref::function _old_function;
      };

      // ******************************************************************* //

      class duplicate_template : public generic
      {
      public:
        duplicate_template ( const id::ref::tmpl& tmpl
                           , const id::ref::tmpl& old_template
                           );
        ~duplicate_template() throw() { }

      private:
        id::ref::tmpl _template;
        id::ref::tmpl _old_template;
      };

      // ******************************************************************* //

      class port_with_unknown_type : public generic
      {
      private:
        std::string nice ( const we::type::PortDirection & direction
                         , const std::string & port
                         , const std::string & type
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << direction << " " << port
            << " with unknown type " << type
            << " in " << path
            ;

          return s.str();
        }
      public:
        port_with_unknown_type ( const we::type::PortDirection & direction
                               , const std::string & port
                               , const std::string & type
                               , const boost::filesystem::path & path
                               )
          : generic (nice (direction, port, type, path))
        {}
      };

      // ******************************************************************* //

      class function_description_with_unknown_port : public generic
      {
      private:
        std::string nice ( const std::string & port_type
                         , const std::string & port_name
                         , const std::string & mod_name
                         , const std::string & mod_function
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << "unknown " << port_type << " port " << port_name
            << " in description of function " << mod_name << "." << mod_function
            << " in " << path
            << std::endl;

          return s.str();
        }
      public:
        function_description_with_unknown_port
        ( const std::string & port_type
        , const std::string & port_name
        , const std::string & mod_name
        , const std::string & mod_function
        , const boost::filesystem::path & path
        )
          : generic (nice (port_type, port_name, mod_name, mod_function, path))
        {}
      };

      // ******************************************************************* //

      class port_connected_place_nonexistent : public generic
      {
      public:
        port_connected_place_nonexistent ( const id::ref::port&
                                         , const boost::filesystem::path&
                                         );
        virtual ~port_connected_place_nonexistent() throw() { }

      private:
        const id::ref::port _port;
        const boost::filesystem::path _path;
      };

      // ******************************************************************* //

      class tunnel_connected_non_virtual : public generic
      {
      public:
        tunnel_connected_non_virtual ( const id::ref::port&
                                     , const id::ref::place&
                                     , const boost::filesystem::path&
                                     );
        virtual ~tunnel_connected_non_virtual() throw() { }

      private:
        const id::ref::port _port;
        const id::ref::place _place;
        const boost::filesystem::path _path;
      };

      // ******************************************************************* //

      class tunnel_name_mismatch : public generic
      {
      public:
        tunnel_name_mismatch ( const id::ref::port&
                             , const id::ref::place&
                             , const boost::filesystem::path&
                             );
        virtual ~tunnel_name_mismatch() throw() { }

      private:
        const id::ref::port _port;
        const id::ref::place _place;
        const boost::filesystem::path _path;
      };

      // ******************************************************************* //

      class port_not_connected : public generic
      {
      public:
        port_not_connected (const id::ref::port&, const boost::filesystem::path&);
        virtual ~port_not_connected() throw() { }

      private:
        const id::ref::port _port;
        const boost::filesystem::path _path;
      };

      // ******************************************************************* //

      class port_connected_type_error : public generic
      {
      public:
        port_connected_type_error ( const id::ref::port&
                                  , const id::ref::place&
                                  , const boost::filesystem::path&
                                  );
        virtual ~port_connected_type_error() throw() { }

      private:
        const id::ref::port _port;
        const id::ref::place _place;
        const boost::filesystem::path _path;
      };

      // ******************************************************************* //

      class port_tunneled_type_error : public generic
      {
      public:
        port_tunneled_type_error ( const std::string& name_virtual
                                 , const signature::type& sig_virtual
                                 , const std::string& name_real
                                 , const signature::type& sig_real
                                 , const boost::filesystem::path& path
                                 )
          : generic
            ( boost::format
              ( "type error: virtual place %1% of type %2%"
              " identified with real place %3% of type %4% in %5%"
              )
              % name_virtual % sig_virtual
              % name_real % sig_real
              % path
            )
        {}
      };

      // ******************************************************************* //

      class connect_to_nonexistent_place : public generic
      {
      private:
        std::string nice ( const std::string & direction
                         , const std::string & trans
                         , const std::string & place
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << "in transition " << trans << " in " << path << ": "
            << "connect-" << direction << " to nonexistent place " << place
            ;

          return s.str();
        }
      public:
        connect_to_nonexistent_place ( const std::string & direction
                                     , const std::string & trans
                                     , const std::string & place
                                     , const boost::filesystem::path & path
                                     )
          : generic (nice (direction, trans, place, path))
        {}
      };

      // ******************************************************************* //

      class connect_to_nonexistent_port : public generic
      {
      private:
        std::string nice ( const std::string & direction
                         , const std::string & trans
                         , const std::string & port
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << "in transition " << trans << " in " << path << ": "
            << "connect-" << direction << " to nonexistent port " << port
            ;

          return s.str();
        }
      public:
        connect_to_nonexistent_port ( const std::string & direction
                                    , const std::string & trans
                                    , const std::string & port
                                    , const boost::filesystem::path & path
                                    )
          : generic (nice (direction, trans, port, path))
        {}
      };

      // ******************************************************************* //

      class unknown_function : public generic
      {
      private:
        std::string nice ( const std::string & fun
                         , const std::string & trans
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << "in transition " << trans << " in " << path << ": "
            << "unknown function " << fun
            ;

          return s.str();
        }
      public:
        unknown_function ( const std::string & fun
                         , const std::string & trans
                         , const boost::filesystem::path & path
                         )
          : generic (nice (fun, trans, path))
        {}
      };

      // ******************************************************************* //

      class unknown_template : public generic
      {
      private:
        std::string nice ( const std::string & templ
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << "unknown template " << templ
            << " in " << path
            ;

          return s.str();
        }
      public:
        unknown_template ( const std::string & templ
                         , const boost::filesystem::path & path
                         )
          : generic (nice (templ, path))
        {}
      };

      // ******************************************************************* //

      class connect_type_error : public generic
      {
      private:
        template<typename PORT, typename PLACE>
        std::string nice ( const std::string & direction
                         , const std::string & trans
                         , const PORT & port
                         , const PLACE & place
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << "in transition " << trans << " in " << path << ": "
            << "type error: connect-" << direction
            << " place " << place.name()
            << " of type " << place.type()
            << " with port " << port.name()
            << " of type " << port.type()
            ;

          return s.str();
        }

      public:
        template<typename PORT, typename PLACE>
        connect_type_error ( const std::string & direction
                           , const std::string trans
                           , const PORT & port
                           , const PLACE & place
                           , const boost::filesystem::path & path
                           )
          : generic (nice (direction, trans, port, place, path))
        {}
      };

      // ******************************************************************* //

      class synthesize_anonymous_function : public generic
      {
      private:
        std::string nice (const boost::filesystem::path & path) const
        {
          std::ostringstream s;

          s << "try to synthesize anonymous function in " << path;

          return s.str();
        }
      public:
        synthesize_anonymous_function (const boost::filesystem::path & path)
          : generic (nice (path))
        {}
      };

      // ******************************************************************* //

      class property_generic : public generic
      {
      private:
        std::string nice ( const std::string & msg
                         , const we::type::property::path_type & key
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << msg << " for property " << fhg::util::join (key, ".")
            << " in " << path
            ;

          return s.str();
        }
      public:
        property_generic ( const std::string & msg
                         , const we::type::property::path_type & key
                         , const boost::filesystem::path & path
                         )
          : generic (nice (msg, key, path))
        {}
      };

      // ******************************************************************* //

      class type_map_mismatch : public generic
      {
      private:
        std::string nice ( const std::string & from
                         , const std::string & to_old
                         , const std::string & to_new
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << "type map mismatch, type " << from
            << " mapped to type " << to_new
            << " and earlier to type " << to_old
            << " in " << path
            ;

          return s.str();
        }
      public:
        type_map_mismatch ( const std::string & from
                          , const std::string & to_old
                          , const std::string & to_new
                          , const boost::filesystem::path & path
                          )
          : generic (nice (from, to_old, to_new, path))
        {}
      };

      // ******************************************************************* //

      class missing_type_out : public generic
      {
      private:
        std::string nice ( const std::string & type
                         , const std::string & spec
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << "missing type-out " << type
            << " in specialization " << spec
            << " in " << path
            ;

          return s.str();
        }
      public:
        missing_type_out ( const std::string & type
                         , const std::string & spec
                         , const boost::filesystem::path & path
                         )
          : generic (nice (type, spec, path))
        {}
      };

      // ******************************************************************* //

      class invalid_prefix : public generic
      {
      private:
        std::string nice ( const std::string & name
                         , const std::string & type
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << type << " " << name
            << " with invalid prefix"
            << " in " << path
            ;

          return s.str();
        }
      public:
        invalid_prefix ( const std::string & name
                       , const std::string & type
                       , const boost::filesystem::path & path
                       )
          : generic (nice (name, type, path))
        {}
      };

      // ******************************************************************* //

      class invalid_name : public generic
      {
      private:
        std::string nice ( const std::string & name
                         , const std::string & type
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << type << " " << name
            << " is invalid (not of the form: [a-zA-Z_][a-zA-Z_0-9]^*)"
            << " in " << path
            ;

          return s.str();
        }
      public:
        invalid_name ( const std::string & name
                     , const std::string & type
                     , const boost::filesystem::path & path
                     )
          : generic (nice (name, type, path))
        {}
      };

      // ******************************************************************* //

      class invalid_field_name : public generic
      {
      private:
        std::string nice ( const std::string & name
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << " invalid field name " << name
            << " in " << path
            ;

          return s.str();
        }
      public:
        invalid_field_name ( const std::string & name
                           , const boost::filesystem::path & path
                           )
          : generic (nice (name, path))
        {}
      };

      // ******************************************************************* //

      class no_map_for_virtual_place : public generic
      {
      private:
        std::string nice ( const std::string & name
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << " missing map for virtual place " << name
            << " in " << path
            ;

          return s.str();
        }
      public:
        no_map_for_virtual_place ( const std::string & name
                                 , const boost::filesystem::path & path
                                 )
          : generic (nice (name, path))
        {}
      };

      // ******************************************************************* //

      class port_type_mismatch : public generic
      {
      public:
        port_type_mismatch ( const id::ref::port& port
                           , const id::ref::port& other_port
                           , const boost::filesystem::path& path
                           );
        virtual ~port_type_mismatch() throw() { }

      private:
        const id::ref::port _port;
        const id::ref::port _other_port;
        const boost::filesystem::path _path;
      };

      // ******************************************************************* //

      class real_place_missing : public generic
      {
      private:
        std::string nice ( const std::string & place_virtual
                         , const std::string & place_real
                         , const std::string & trans
                         , const boost::filesystem::path & path
                         ) const
        {
          std::ostringstream s;

          s << " missing real place " << place_real
            << " to replace virtual place " << place_virtual
            << " in transition " << trans
            << " in " << path
            ;

          return s.str();
        }
      public:
        real_place_missing ( const std::string & place_virtual
                           , const std::string & place_real
                           , const std::string & trans
                           , const boost::filesystem::path & path
                           )
          : generic (nice (place_virtual, place_real, trans, path))
        {}
      };

      // ******************************************************************* //

      namespace parse_function
      {
        class formatted : public generic
        {
        private:
          std::string nice ( const std::string & name
                           , const std::string & function
                           , const std::string & what
                           , const std::size_t & k
                           , const boost::filesystem::path & path
                           ) const
          {
            std::ostringstream s;

            s << "error while parsing a function description for module"
              << " name " << name << " in " << path << ":"
              << std::endl << function
              << std::endl
              ;
            for (std::size_t i (0); i < k; ++i) { s << " "; }
            s << "^" << std::endl;
            s << what << std::endl;

            return s.str();
          }
        public:
          formatted ( const std::string & name
                    , const std::string & function
                    , const std::string & what
                    , const std::size_t & k
                    , const boost::filesystem::path & path
                    )
            : generic (nice (name, function, what, k, path))
          {}
        };

        class expected : public formatted
        {
        public:
          expected ( const std::string & name
                   , const std::string & function
                   , const std::string & what
                   , const std::size_t & k
                   , const boost::filesystem::path & path
                   )
            : formatted ( name
                        , function
                        , "expected " + fhg::util::show (what)
                        , k
                        , path
                        )
          {}
        };
      }

      // ******************************************************************* //

      class could_not_open_file : public generic
      {
      private:
        std::string nice (const std::string & file) const
        {
          std::ostringstream s;

          s << "could not open file " << file;

          return s.str();
        }

      public:
        could_not_open_file (const boost::filesystem::path & file)
          : generic (nice (file.string()))
        {}

        could_not_open_file (const std::string & file)
          : generic (nice (file))
        {}
      };

      // ******************************************************************* //

      class could_not_create_directory : public generic
      {
      private:
        std::string nice (const boost::filesystem::path & path) const
        {
          std::ostringstream s;

          s << "could not create directory " << path;

          return s.str();
        }

      public:
        could_not_create_directory (const boost::filesystem::path & path)
          : generic (nice (path))
        {}
      };

      // ******************************************************************* //

      class duplicate_external_function : public generic
      {
      private:
        std::string nice ( const std::string & name
                         , const std::string & mod
                         , const boost::filesystem::path & file1
                         , const boost::filesystem::path & file2
                         ) const
        {
          std::ostringstream s;

          s << "the external function " << name
            << " in module " << mod
            << " has different definitions"
            << " in " << file1 << " and in " << file2
            ;

          return s.str();
        }
      public:
        duplicate_external_function ( const std::string & name
                                    , const std::string & mod
                                    , const boost::filesystem::path & file1
                                    , const boost::filesystem::path & file2
                                    )
          : generic (nice (name, mod, file1, file2))
        {}
      };

      // ******************************************************************* //

      class template_without_function : public generic
      {
      public:
        template_without_function ( const boost::optional<std::string>& name
                                  , const boost::filesystem::path& path
                                  )
          : generic ( boost::format
                      ("template %1% without a function in %2%") % name % path
                    )
        {}
      };

      // ******************************************************************* //

      class weparse : public generic
      {
      public:
        weparse (const std::string & msg)
          : generic (msg)
        {}
      };

      // ******************************************************************* //

      class strange : public generic
      {
      public:
        strange (const std::string & msg)
          : generic ("this is STRANGE and should not happen", msg)
        {}
      };

#define THROW_STRANGE(msg) do { std::ostringstream s; s << __FILE__ << " [" << __LINE__ << "]: " << msg; throw error::strange (s.str()); } while (0)
    }
  }
}

#endif
