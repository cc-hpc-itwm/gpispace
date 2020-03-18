// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#pragma once

#include <string>

#include <xml/parse/type/connect.fwd.hpp>
#include <xml/parse/type/function.fwd.hpp>
#include <xml/parse/type/memory_buffer.fwd.hpp>
#include <xml/parse/type/mod.fwd.hpp>
#include <xml/parse/type/net.fwd.hpp>
#include <xml/parse/type/place.fwd.hpp>
#include <xml/parse/type/place_map.fwd.hpp>
#include <xml/parse/type/port.fwd.hpp>
#include <xml/parse/type/response.fwd.hpp>
#include <xml/parse/type/eureka.fwd.hpp>
#include <xml/parse/type/specialize.fwd.hpp>
#include <xml/parse/type/struct.fwd.hpp>
#include <xml/parse/type/template.fwd.hpp>
#include <xml/parse/type/transition.fwd.hpp>
#include <xml/parse/util/position.hpp>

#include <we/type/port.hpp>
#include <we/type/property.hpp>

#include <we/type/signature.hpp>
#include <we/type/signature/show.hpp>

#include <fhg/util/boost/optional.hpp>
#include <util-generic/join.hpp>

#include <rapidxml.hpp>

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
        place_type_unknown (const type::place_type&);
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

      template<typename T>
      class generic_duplicate : public generic
      {
      public:
        generic_duplicate ( const T& early
                          , const T& late
                          , const boost::format& fmt
                          )
          : generic ( boost::format ( "duplicate %1% at %2%"
                                      ", earlier definition is at %3%"
                                    )
                    % fmt
                    % late.position_of_definition()
                    % early.position_of_definition()
                    )
        {}
      };

#define DUPLICATE(_name, _type)                                         \
      class duplicate_ ## _name : public generic_duplicate<_type>       \
      {                                                                 \
      public:                                                           \
        duplicate_ ## _name (const _type& early, const _type& late);    \
      }

      DUPLICATE (connect, type::connect_type);
      DUPLICATE (external_function, type::module_type);
      DUPLICATE (memory_buffer, type::memory_buffer_type);
      DUPLICATE (place, type::place_type);
      DUPLICATE (place_map, type::place_map_type);
      DUPLICATE (port, type::port_type);
      DUPLICATE (response, type::response_type);
      DUPLICATE (eureka, type::eureka_type);
      DUPLICATE (specialize, type::specialize_type);
      DUPLICATE (template, type::tmpl_type);
      DUPLICATE (transition, type::transition_type);

#undef DUPLICATE

      // ******************************************************************* //

      class empty_preferences : public generic
      {
      public:
        empty_preferences (const util::position_type&);
      };

      class duplicate_preference : public generic
      {
      public:
        duplicate_preference (const std::string&, const util::position_type&);
      };

      class preferences_without_modules : public generic
      {
      public:
        preferences_without_modules (const util::position_type&);
      };

      class missing_target_for_module : public generic
      {
      public:
        missing_target_for_module ( const std::string&
                                  , const util::position_type&
                                  );
      };

      class modules_without_preferences : public generic
      {
      public:
        modules_without_preferences ( const std::string&
                                    , const std::string&
                                    , const util::position_type&
                                    );
      };

      class duplicate_module_for_target : public generic
      {
      public:
        duplicate_module_for_target ( const std::string&
                                     , const std::string&
                                     , const util::position_type&
                                     );
      };

      class mismatching_modules_and_preferences : public generic
      {
      public:
        mismatching_modules_and_preferences ( const std::list<std::string>&
                                             , const std::list<std::string>&
                                             , const util::position_type&
                                             );
      };

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
        port_connected_place_nonexistent ( const type::port_type&
                                         , const boost::filesystem::path&
                                         );

      private:
        const boost::filesystem::path _path;
      };

      // ******************************************************************* //

      class tunnel_connected_non_virtual : public generic
      {
      public:
        tunnel_connected_non_virtual ( const type::port_type&
                                     , const type::place_type&
                                     , const boost::filesystem::path&
                                     );

      private:
        const boost::filesystem::path _path;
      };

      // ******************************************************************* //

      class tunnel_name_mismatch : public generic
      {
      public:
        tunnel_name_mismatch ( const type::port_type&
                             , const type::place_type&
                             , const boost::filesystem::path&
                             );

      private:
        const boost::filesystem::path _path;
      };

      // ******************************************************************* //

      class port_not_connected : public generic
      {
      public:
        port_not_connected (const type::port_type&, const boost::filesystem::path&);

      private:
        const boost::filesystem::path _path;
      };

      // ******************************************************************* //

      class port_connected_type_error : public generic
      {
      public:
        port_connected_type_error ( const type::port_type&
                                  , const type::place_type&
                                  , const boost::filesystem::path&
                                  );

      private:
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
        connect_to_nonexistent_place ( type::transition_type const&
                                     , const type::connect_type&
                                     );
      };

      class connect_to_nonexistent_port : public generic
      {
      public:
        connect_to_nonexistent_port ( type::transition_type const&
                                    , const type::connect_type&
                                    );
      };

      // ******************************************************************* //

      class unknown_function : public generic
      {
      public:
        unknown_function (const std::string&, type::transition_type const&);

      private:
        const std::string _function_name;
      };

      // ******************************************************************* //

      class unknown_port_in_connect_response : public generic
      {
      public:
        unknown_port_in_connect_response (type::response_type const&);
      };

      class unknown_to_in_connect_response : public generic
      {
      public:
        unknown_to_in_connect_response (type::response_type const&);
      };

      class invalid_signature_in_connect_response : public generic
      {
      public:
        invalid_signature_in_connect_response
          ( type::response_type const&
          , type::port_type const&
          );
      };

      // ******************************************************************* //

      class unknown_template : public generic
      {
      public:
        unknown_template (type::specialize_type const&, type::net_type const&);
      };

      // ******************************************************************* //

      class connect_type_error : public generic
      {
      public:
        connect_type_error ( type::transition_type const&
                           , const type::connect_type&
                           , const type::port_type&
                           , const type::place_type&
                           );
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
        memory_buffer_for_non_module (type::function_type const&);
        ~memory_buffer_for_non_module() throw() = default;
      };

      class memory_transfer_for_non_module : public generic
      {
      public:
        memory_transfer_for_non_module (type::function_type const&);
        ~memory_transfer_for_non_module() throw() = default;
      };

      class memory_buffer_with_same_name_as_port : public generic
      {
      public:
        memory_buffer_with_same_name_as_port
          (type::memory_buffer_type const&, type::port_type const&);
        virtual ~memory_buffer_with_same_name_as_port() throw() = default;
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
        port_type_mismatch ( const type::port_type& port
                           , const type::port_type& other_port
                           );
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
