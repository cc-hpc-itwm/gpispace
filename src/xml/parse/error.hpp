// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#pragma once

#include <string>

#include <xml/parse/id/types.hpp>
#include <xml/parse/util/position.hpp>

#include <we/type/port.hpp>
#include <we/type/property.hpp>

#include <we/type/signature.hpp>
#include <we/type/signature/show.hpp>

#include <fhg/util/boost/optional.hpp>
#include <util-generic/join.hpp>

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
      class generic : public std::runtime_error
      {
      public:
        generic (const std::string & msg)
          : std::runtime_error ("ERROR: " + msg)
        { }

        generic (const boost::format& bf)
          : generic (bf.str())
        { }

        generic (const std::string & msg, const std::string & pre)
          : generic (pre + ": " + msg)
        { }

        generic ( boost::format const& format
                , boost::filesystem::path const& path
                )
          : generic (boost::format ("%1% in %2%") % format % path)
        {}

        generic ( boost::format const& format
                , util::position_type const& position
                )
          : generic (boost::format ("%1% in %2%") % format % position)
        {}
      };

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
      public:
        explicit file_already_there (const boost::filesystem::path& file)
          : generic
            ( boost::format ("file %1% already there with a different content")
            % file
            )
        {}
      };

      // ******************************************************************* //

      template<typename IT>
      class include_loop : public generic
      {
      public:
        include_loop ( const std::string & pre
                     , IT pos, const IT & end
                     )
          : generic (pre, "include loop: " + fhg::util::join (pos, end, " -> ").string())
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
      };

      class struct_field_redefined : public generic
      {
      public:
        struct_field_redefined ( const std::string& name
                               , const boost::filesystem::path& path
                               )
          : generic ( boost::format ("struct field '%1%' redefined")
                    % name , path
                    )
        {}
      };

      class place_type_unknown : public generic
      {
      public:
        place_type_unknown (const id::ref::place&);

      private:
        const id::ref::place _place;
      };

      // ******************************************************************* //

      class parse_lift : public generic
      {
      public:
        parse_lift ( const std::string & place
                   , const std::string & field
                   , const boost::filesystem::path & path
                   , const std::string & msg
                   )
          : generic
            ( boost::format
              ("when reading a value for place %1%%2% from %3%: %4%")
            % place
            % (field.empty() ? "" : (" for field " + field))
            % path
            % msg
            )
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
      DUPLICATE (response);
      DUPLICATE (memory_buffer);

#undef DUPLICATE
#undef DUPLICATE_WITH_ID

      // ******************************************************************* //

      class port_with_unknown_type : public generic
      {
      public:
        port_with_unknown_type ( const we::type::PortDirection & direction
                               , const std::string & port
                               , const std::string & type
                               , const boost::filesystem::path & path
                               )
          : generic
            ( boost::format ("%1% %2% with unknown type %3%")
            % direction
            % port
            % type
            , path
            )
        {}
      };

      // ******************************************************************* //

      class function_description_with_unknown_port : public generic
      {
      public:
        function_description_with_unknown_port
        ( const std::string & port_type
        , const std::string & port_name
        , const std::string & mod_name
        , const std::string & mod_function
        , const boost::filesystem::path & path
        )
          : generic
            ( boost::format
              ("unknown %1% port %2% in description of function %3%.%4%")
            % port_type
            % port_name
            % mod_name
            % mod_function
            , path
            )
        {}
      };

      // ******************************************************************* //

      class port_connected_place_nonexistent : public generic
      {
      public:
        port_connected_place_nonexistent ( const id::ref::port&
                                         , const boost::filesystem::path&
                                         );

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
                                 , const pnet::type::signature::signature_type& sig_virtual
                                 , const std::string& name_real
                                 , const pnet::type::signature::signature_type& sig_real
                                 , const boost::filesystem::path& path
                                 )
          : generic
            ( boost::format
              ( "type error: virtual place %1% of type %2%"
              " identified with real place %3% of type %4%"
              )
            % name_virtual
            % pnet::type::signature::show (sig_virtual)
            % name_real
            % pnet::type::signature::show (sig_real)
            , path
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

      private:
        const id::ref::transition _transition;
        const id::ref::connect _connection;
      };

      // ******************************************************************* //

      class unknown_function : public generic
      {
      public:
        unknown_function (const std::string&, const id::ref::transition&);

      private:
        const std::string _function_name;
        const id::ref::transition _transition;
      };

      // ******************************************************************* //

      class unknown_port_in_connect_response : public generic
      {
      public:
        unknown_port_in_connect_response (id::ref::response const&);
      };

      class unknown_to_in_connect_response : public generic
      {
      public:
        unknown_to_in_connect_response (id::ref::response const&);
      };

      class invalid_signature_in_connect_response : public generic
      {
      public:
        invalid_signature_in_connect_response ( id::ref::response const&
                                              , id::ref::port const&
                                              );
      };

      // ******************************************************************* //

      class unknown_template : public generic
      {
      public:
        unknown_template (const id::ref::specialize&, const id::ref::net&);

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

      private:
        const id::ref::transition _transition;
        const id::ref::connect _connection;
        const id::ref::port _port;
        const id::ref::place _place;
      };

      class memory_buffer_without_size : public generic
      {
      public:
        memory_buffer_without_size ( std::string const&
                                   , util::position_type const&
                                   );
        virtual ~memory_buffer_without_size() throw() = default;

      private:
        std::string const _name;
        util::position_type const _position_of_definition;
      };

      class memory_buffer_for_non_module : public generic
      {
      public:
        memory_buffer_for_non_module (id::ref::function const&);
        ~memory_buffer_for_non_module() throw() = default;

      private:
        id::ref::function const _function;
      };

      class memory_transfer_for_non_module : public generic
      {
      public:
        memory_transfer_for_non_module (id::ref::function const&);
        ~memory_transfer_for_non_module() throw() = default;

      private:
        id::ref::function const _function;
      };

      class memory_buffer_with_same_name_as_port : public generic
      {
      public:
        memory_buffer_with_same_name_as_port
          (id::ref::memory_buffer const&, id::ref::port const&);
        virtual ~memory_buffer_with_same_name_as_port() throw() = default;

      private:
        id::ref::memory_buffer const _memory_buffer;
        id::ref::port const _port;
      };

      // ******************************************************************* //

      class property_generic : public generic
      {
      public:
        property_generic ( const std::string & msg
                         , const we::type::property::path_type & key
                         , const boost::filesystem::path & path
                         )
          : generic
            ( boost::format ("%1% for property %2%")
            % msg
            % fhg::util::join (key, '.')
            , path
            )
        {}
      };

      // ******************************************************************* //

      class type_map_mismatch : public generic
      {
      public:
        type_map_mismatch ( const std::string & from
                          , const std::string & to_old
                          , const std::string & to_new
                          , const boost::filesystem::path & path
                          )
          : generic
            ( boost::format ("type map mismatch, type %1% mapped to type %3%"
                            " and earlier to type %3%"
                            )
            % from
            % to_old
            % to_new
            , path
            )
        {}
      };

      // ******************************************************************* //

      class missing_type_out : public generic
      {
      public:
        missing_type_out ( const std::string & type
                         , const std::string & spec
                         , const boost::filesystem::path & path
                         )
          : generic
            ( boost::format ("missing type-out %1% in specialization %2%")
            % type
            % spec
            , path
            )
        {}
      };

      // ******************************************************************* //

      class invalid_prefix : public generic
      {
      public:
        invalid_prefix ( const std::string & name
                       , const std::string & type
                       , const boost::filesystem::path & path
                       )
          : generic
            ( boost::format ("%2% %1% with invalid prefix")
            % name
            % type
            , path
            )
        {}
      };

      // ******************************************************************* //

      class invalid_name : public generic
      {
      public:
        invalid_name ( const std::string & name
                     , const std::string & type
                     , const boost::filesystem::path & path
                     )
          : generic
            ( boost::format
              ("%2% %1% is invalid (not of the form: [a-zA-Z_][a-zA-Z_0-9]^*)")
            % name
            % type
            , path
            )
        {}
      };

      // ******************************************************************* //

      class invalid_field_name : public generic
      {
      public:
        invalid_field_name ( const std::string & name
                           , const boost::filesystem::path & path
                           )
          : generic (boost::format (" invalid field name %1%") % name, path)
        {}
      };

      // ******************************************************************* //

      class no_map_for_virtual_place : public generic
      {
      public:
        no_map_for_virtual_place ( const std::string & name
                                 , const boost::filesystem::path & path
                                 )
          : generic
            ( boost::format (" missing map for virtual place %1%") % name
            , path
            )
        {}
      };

      // ******************************************************************* //

      class port_type_mismatch : public generic
      {
      public:
        port_type_mismatch ( const id::ref::port& port
                           , const id::ref::port& other_port
                           );

      private:
        const id::ref::port _port;
        const id::ref::port _other_port;
      };

      // ******************************************************************* //

      class real_place_missing : public generic
      {
      public:
        real_place_missing ( const std::string & place_virtual
                           , const std::string & place_real
                           , const std::string & trans
                           , const boost::filesystem::path & path
                           )
          : generic
            ( boost::format
              (" missing real place %2% to replace virtual place %1%"
              " in transition %3%"
              )
            % place_virtual
            % place_real
            % trans
            , path
            )
        {}
      };

      // ******************************************************************* //

      namespace parse_function
      {
        class formatted : public generic
        {
        public:
          formatted ( const std::string & name
                    , const std::string & function
                    , const std::string & what
                    , const std::size_t & k
                    , const boost::filesystem::path & path
                    )
            : generic
              ( boost::format
                (R"EOS(error while parsing a function description for module name %1% in %5%:
%2%
%4%^
%3%
)EOS"
                )
              % name
              % function
              % what
              % std::string (k, ' ')
              % path
              )
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
                        , "expected " + what
                        , k
                        , path
                        )
          {}
        };
      }

      // ******************************************************************* //

      class could_not_open_file : public generic
      {
      public:
        could_not_open_file (const boost::filesystem::path & file)
          : could_not_open_file (file.string())
        {}

        could_not_open_file (const std::string & file)
          : generic (boost::format ("could not open file %1%") % file)
        {}
      };

      // ******************************************************************* //

      class could_not_create_directory : public generic
      {
      public:
        could_not_create_directory (const boost::filesystem::path & path)
          : generic (boost::format ("could not create directory %1%") % path)
        {}
      };

      // ******************************************************************* //

      class template_without_function : public generic
      {
      public:
        template_without_function ( const boost::optional<std::string>& name
                                  , const boost::filesystem::path& path
                                  )
          : generic
            (boost::format ("template %1% without a function") % name, path)
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
