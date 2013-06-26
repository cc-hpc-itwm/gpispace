// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _XML_PARSE_ERROR_HPP
#define _XML_PARSE_ERROR_HPP

#include <string>
#include <sstream>

#include <xml/parse/id/types.hpp>
#include <xml/parse/util/position.hpp>

#include <we/type/literal.hpp>
#include <we/type/signature.hpp>
#include <we/type/port.hpp>
#include <we/type/property.hpp>

#include <fhg/util/join.hpp>
#include <fhg/util/backtracing_exception.hpp>
#include <fhg/util/boost/optional.hpp>

#include <xml/parse/rapidxml/1.13/rapidxml.hpp>

#include <xml/parse/util/show_node_type.hpp>

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
      public:
        wrong_node ( const rapidxml::node_type&
                   , const rapidxml::node_type& got
                   , const util::position_type&
                   );

        wrong_node ( const rapidxml::node_type&
                   , const rapidxml::node_type&
                   , const rapidxml::node_type& got
                   , const util::position_type&
                   );
      };

      // ******************************************************************* //

     class missing_attr : public generic
     {
     public:
       missing_attr ( const std::string& pre
                    , const std::string& attr
                    , const util::position_type&
                    );
      };

      // ******************************************************************* //

      class no_elements_given : public generic
      {
      public:
        no_elements_given (const std::string&, const boost::filesystem::path&);
      };

      // ******************************************************************* //

      class more_than_one_definition : public generic
      {
      public:
        more_than_one_definition
          (const std::string&, const util::position_type&);
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
      public:
        cannot_resolve ( const std::string& field
                       , const std::string& type
                       , const type::structure_type&
                       );
        virtual ~cannot_resolve() throw() {}

      private:
        const std::string _field;
        const std::string _type;
      };

      class struct_redefined : public generic
      {
      public:
        struct_redefined ( const type::structure_type& early
                         , const type::structure_type& late
                         );
        virtual ~struct_redefined() throw() {}
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

      class place_type_unknown : public generic
      {
      public:
        place_type_unknown (const id::ref::place&);
        virtual ~place_type_unknown() throw() {}

      private:
        const id::ref::place _place;
      };

      // ******************************************************************* //

      class parse_type_mismatch : public generic
      {
      private:
        std::string nice  ( const std::string & place
                          , const std::string & field
                          , const std::string & sig
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
                          , const std::string & val
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
                            , const std::string & sig
                            , const signature::structured_t & val
                            , const boost::filesystem::path & path
                            )
          : generic (nice (place, field, sig, val, path))
        {}

        parse_type_mismatch ( const std::string & place
                            , const std::string & field
                            , const signature::structured_t & sig
                            , const std::string & val
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

      class forbidden_shadowing : public generic
      {
      public:
        forbidden_shadowing ( const type::structure_type& early
                            , const type::structure_type& late
                            , const std::string& port_name
                            );
        virtual ~forbidden_shadowing() throw() {}
      };

      // ******************************************************************* //

      class parse_link_prefix : public generic
      {
      public:
        parse_link_prefix ( const std::string& reason
                          , const std::string& input
                          , const std::size_t& pos
                          );
        virtual ~parse_link_prefix() throw() {}

      private:
        const std::string _reason;
        const std::string _input;
        //        const std::size_t _pos;
      };

      class link_prefix_missing : public generic
      {
      public:
        link_prefix_missing (const std::string&);
        virtual ~link_prefix_missing() throw() {}
      private:
        const std::string _key;
      };

      // ******************************************************************* //

      template<typename Id>
      class generic_duplicate : public generic
      {
      public:
        generic_duplicate ( const Id& early
                          , const Id& late
                          , const boost::format& fmt
                          )
          : generic ( boost::format ( "duplicate %1% at %2%"
                                      ", earlier definition is at %3%"
                                    )
                    % fmt
                    % late.get().position_of_definition()
                    % early.get().position_of_definition()
                    )
          , _early (early)
          , _late (late)
        {}

        virtual ~generic_duplicate() throw() {}

        const Id& early() const
        {
          return _early;
        }
        const Id& late() const
        {
          return _late;
        }
      private:
        const Id _early;
        const Id _late;
      };

#define DUPLICATE_WITH_ID(_type,_id)                                    \
      class duplicate_ ##_type : public generic_duplicate<id::ref::_id> \
      {                                                                 \
      public:                                                           \
        duplicate_ ##_type ( const id::ref::_id& early                  \
                           , const id::ref::_id& late                   \
                           );                                           \
      }

#define DUPLICATE(_type) DUPLICATE_WITH_ID(_type,_type)

      DUPLICATE (specialize);
      DUPLICATE (place);
      DUPLICATE (transition);
      DUPLICATE (port);
      DUPLICATE_WITH_ID (template,tmpl);
      DUPLICATE (place_map);
      DUPLICATE_WITH_ID (external_function,module);
      DUPLICATE (connect);

#undef DUPLICATE
#undef DUPLICATE_WITH_ID

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
      public:
        connect_to_nonexistent_place ( const id::ref::transition&
                                     , const id::ref::connect&
                                     );
        virtual ~connect_to_nonexistent_place() throw() {}

      private:
        const id::ref::transition _transition;
        const id::ref::connect _connection;
      };

      class connect_to_nonexistent_port : public generic
      {
      public:
        connect_to_nonexistent_port ( const id::ref::transition&
                                    , const id::ref::connect&
                                    );
        virtual ~connect_to_nonexistent_port() throw() {}

      private:
        const id::ref::transition _transition;
        const id::ref::connect _connection;
      };

      // ******************************************************************* //

      class unknown_function : public generic
      {
      public:
        unknown_function (const std::string&, const id::ref::transition&);
        virtual ~unknown_function() throw() {}

      private:
        const std::string _function_name;
        const id::ref::transition _transition;
      };

      // ******************************************************************* //

      class unknown_template : public generic
      {
      public:
        unknown_template (const id::ref::specialize&, const id::ref::net&);
        virtual ~unknown_template() throw() {}

      private:
        const id::ref::specialize _specialize;
        const id::ref::net _net;
      };

      // ******************************************************************* //

      class connect_type_error : public generic
      {
      public:
        connect_type_error ( const id::ref::transition&
                           , const id::ref::connect&
                           , const id::ref::port&
                           , const id::ref::place&
                           );
        virtual ~connect_type_error() throw() {}

      private:
        const id::ref::transition _transition;
        const id::ref::connect _connection;
        const id::ref::port _port;
        const id::ref::place _place;
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
                           );
        virtual ~port_type_mismatch() throw() { }

      private:
        const id::ref::port _port;
        const id::ref::port _other_port;
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
    }
  }
}

#endif
