// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include <xml/parse/type/connect.fwd.hpp>
#include <xml/parse/type/eureka.fwd.hpp>
#include <xml/parse/type/function.fwd.hpp>
#include <xml/parse/type/memory_buffer.fwd.hpp>
#include <xml/parse/type/mod.fwd.hpp>
#include <xml/parse/type/net.fwd.hpp>
#include <xml/parse/type/place.fwd.hpp>
#include <xml/parse/type/place_map.fwd.hpp>
#include <xml/parse/type/port.fwd.hpp>
#include <xml/parse/type/response.fwd.hpp>
#include <xml/parse/type/specialize.fwd.hpp>
#include <xml/parse/type/struct.fwd.hpp>
#include <xml/parse/type/template.fwd.hpp>
#include <xml/parse/type/transition.fwd.hpp>
#include <xml/parse/util/position.hpp>

#include <we/type/Port.hpp>
#include <we/type/eureka.hpp>
#include <we/type/property.hpp>

#include <we/type/signature.hpp>
#include <we/type/signature/show.hpp>

#include <util-generic/join.hpp>

#include <rapidxml.hpp>

#include <xml/parse/util/show_node_type.hpp>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>

#include <FMT/boost/filesystem/path.hpp>
#include <FMT/boost/optional.hpp>
#include <FMT/boost/variant.hpp>
#include <FMT/util-generic/join.hpp>
#include <FMT/we/type/port/direction.hpp>
#include <FMT/we/type/signature/show.hpp>
#include <FMT/xml/parse/util/position.hpp>
#include <fmt/core.h>

namespace xml
{
  namespace parse
  {
    namespace error
    {
      class generic : public std::runtime_error
      {
      public:
        generic (std::string const& msg)
          : std::runtime_error ("ERROR: " + msg)
        { }

        generic (std::string const& msg, std::string const& pre)
          : generic (pre + ": " + msg)
        { }

        generic ( std::string const& message
                , ::boost::filesystem::path const& path
                )
          : generic {fmt::format ("{} in {}", message, path)}
        {}

        generic ( std::string const& format
                , util::position_type const& position
                )
          : generic {fmt::format ("{} in {}", format, position)}
        {}
      };

      // ******************************************************************* //

      class wrong_node : public generic
      {
      public:
        wrong_node ( rapidxml::node_type const&
                   , rapidxml::node_type const& got
                   , util::position_type const&
                   );

        wrong_node ( rapidxml::node_type const&
                   , rapidxml::node_type const&
                   , rapidxml::node_type const& got
                   , util::position_type const&
                   );
      };

      // ******************************************************************* //

     class missing_attr : public generic
     {
     public:
       missing_attr ( std::string const& pre
                    , std::string const& attr
                    , util::position_type const&
                    );
      };

      // ******************************************************************* //

      class no_elements_given : public generic
      {
      public:
        no_elements_given (std::string const&, ::boost::filesystem::path const&);
      };

      // ******************************************************************* //

      class more_than_one_definition : public generic
      {
      public:
        more_than_one_definition
          (std::string const&, util::position_type const&);
      };

      // ******************************************************************* //

      class top_level_anonymous_function : public generic
      {
      public:
        top_level_anonymous_function ( std::string const& file
                                     , std::string const& pre
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
        top_level_anonymous_template ( std::string const& file
                                     , std::string const& pre
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
        file_not_found ( std::string const& pre
                       , std::string const& file
                       )
          : generic (pre, "file not found: " + file)
        {}
      };

      // ******************************************************************* //

      class file_already_there : public generic
      {
      public:
        explicit file_already_there (::boost::filesystem::path const& file)
          : generic
            { fmt::format ( "file {} already there with a different content"
                          , file
                          )
            }
        {}
      };

      // ******************************************************************* //

      template<typename IT>
      class include_loop : public generic
      {
      public:
        include_loop ( std::string const& pre
                     , IT pos, IT const& end
                     )
          : generic (pre, "include loop: " + fhg::util::join (pos, end, " -> ").string())
        {}
      };

      // ******************************************************************* //

      class cannot_resolve : public generic
      {
      public:
        cannot_resolve ( std::string const& field
                       , std::string const& type
                       , type::structure_type const&
                       );

      private:
        const std::string _field;
        const std::string _type;
      };

      class struct_redefined : public generic
      {
      public:
        struct_redefined ( type::structure_type const& early
                         , type::structure_type const& late
                         );
      };

      class struct_field_redefined : public generic
      {
      public:
        struct_field_redefined ( std::string const& name
                               , ::boost::filesystem::path const& path
                               )
          : generic { fmt::format ("struct field '{}' redefined", name)
                    , path
                    }
        {}
      };

      class place_type_unknown : public generic
      {
      public:
        place_type_unknown (type::place_type const&);
      };

      // ******************************************************************* //

      class parse_lift : public generic
      {
      public:
        parse_lift ( std::string const& place
                   , std::string const& field
                   , ::boost::filesystem::path const& path
                   , std::string const& msg
                   )
          : generic
            { fmt::format
              ( "when reading a value for place {}{} from {}: {}"
              , place
              , field.empty() ? "" : (" for field " + field)
              , path
              , msg
              )
            }
        {}
      };

      // ******************************************************************* //

      class eval_context_bind : public generic
      {
      public:
        eval_context_bind ( std::string const& descr
                          , std::string const& msg
                          )
          : generic (descr + ": " + msg)
        {}
      };

      // ******************************************************************* //

      template<typename T>
      class generic_duplicate : public generic
      {
      public:
        generic_duplicate ( T const& early
                          , T const& late
                          , std::string const& fmt
                          )
          : generic { fmt::format ( "duplicate {} at {}"
                                    ", earlier definition is at {}"
                                  , fmt
                                  , late.position_of_definition()
                                  , early.position_of_definition()
                                  )
                    }
        {}
      };

#define DUPLICATE(_name, _type)                                         \
      class duplicate_ ## _name : public generic_duplicate<_type>       \
      {                                                                 \
      public:                                                           \
        duplicate_ ## _name (_type const& early, _type const& late);    \
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
        empty_preferences (util::position_type const&);
      };

      class duplicate_preference : public generic
      {
      public:
        duplicate_preference (std::string const&, util::position_type const&);
      };

      class preferences_without_modules : public generic
      {
      public:
        preferences_without_modules (util::position_type const&);
      };

      class missing_target_for_module : public generic
      {
      public:
        missing_target_for_module ( std::string const&
                                  , util::position_type const&
                                  );
      };

      class modules_without_preferences : public generic
      {
      public:
        modules_without_preferences (util::position_type const&);

        modules_without_preferences ( std::string const&
                                    , std::string const&
                                    , util::position_type const&
                                    );
      };

      class duplicate_module_for_target : public generic
      {
      public:
        duplicate_module_for_target ( std::string const&
                                    , std::string const&
                                    , util::position_type const&
                                    );
      };

      class mismatching_modules_and_preferences : public generic
      {
      public:
        mismatching_modules_and_preferences ( std::list<std::string> const&
                                            , std::list<std::string> const&
                                            , util::position_type const&
                                            );
      };

      // ******************************************************************* //

      class mismatching_eureka_for_module : public generic
      {
      public:
        mismatching_eureka_for_module ( std::string const&
                                      , util::position_type const&
                                      );
      };

      // ******************************************************************* //

      class eureka_group_attribute_and_tag : public generic
      {
      public:
        eureka_group_attribute_and_tag
          ( std::string const& module_name
          , we::type::eureka_id_type const& id_attribute
          , util::position_type const& pod_attribute
          , we::type::eureka_id_type const& id_tag
          , util::position_type const& pod_tag
          );
      };

      // ******************************************************************* //

      class port_with_unknown_type : public generic
      {
      public:
        port_with_unknown_type ( we::type::PortDirection const& direction
                               , std::string const& port
                               , std::string const& type
                               , ::boost::filesystem::path const& path
                               )
          : generic
            { fmt::format ( "{} {} with unknown type {}"
                          , direction
                          , port
                          , type
                          )
            , path
            }
        {}
      };

      // ******************************************************************* //

      class function_description_with_unknown_port : public generic
      {
      public:
        function_description_with_unknown_port
        ( std::string const& port_type
        , std::string const& port_name
        , std::string const& mod_name
        , std::string const& mod_function
        , ::boost::filesystem::path const& path
        )
          : generic
            { fmt::format
              ( "unknown {} port {} in description of function {}.{}"
              , port_type
              , port_name
              , mod_name
              , mod_function
              )
            , path
            }
        {}
      };

      // ******************************************************************* //

      class port_connected_place_nonexistent : public generic
      {
      public:
        port_connected_place_nonexistent ( type::port_type const&
                                         , ::boost::filesystem::path const&
                                         );

      private:
        const ::boost::filesystem::path _path;
      };

      // ******************************************************************* //

      class tunnel_connected_non_virtual : public generic
      {
      public:
        tunnel_connected_non_virtual ( type::port_type const&
                                     , type::place_type const&
                                     , ::boost::filesystem::path const&
                                     );

      private:
        const ::boost::filesystem::path _path;
      };

      // ******************************************************************* //

      class tunnel_name_mismatch : public generic
      {
      public:
        tunnel_name_mismatch ( type::port_type const&
                             , type::place_type const&
                             , ::boost::filesystem::path const&
                             );

      private:
        const ::boost::filesystem::path _path;
      };

      // ******************************************************************* //

      class port_not_connected : public generic
      {
      public:
        port_not_connected (type::port_type const&, ::boost::filesystem::path const&);

      private:
        const ::boost::filesystem::path _path;
      };

      // ******************************************************************* //

      class port_connected_type_error : public generic
      {
      public:
        port_connected_type_error ( type::port_type const&
                                  , type::place_type const&
                                  , ::boost::filesystem::path const&
                                  );

      private:
        const ::boost::filesystem::path _path;
      };

      // ******************************************************************* //

      class port_tunneled_type_error : public generic
      {
      public:
        port_tunneled_type_error ( std::string const& name_virtual
                                 , pnet::type::signature::signature_type const& sig_virtual
                                 , std::string const& name_real
                                 , pnet::type::signature::signature_type const& sig_real
                                 , ::boost::filesystem::path const& path
                                 )
          : generic
            { fmt::format
              ( "type error: virtual place {} of type {}"
                " identified with real place {} of type {}"
              , name_virtual
              , pnet::type::signature::show (sig_virtual)
              , name_real
              , pnet::type::signature::show (sig_real)
              )
            , path
            }
        {}
      };

      // ******************************************************************* //

      class connect_to_nonexistent_place : public generic
      {
      public:
        connect_to_nonexistent_place ( type::transition_type const&
                                     , type::connect_type const&
                                     );
      };

      class connect_to_nonexistent_port : public generic
      {
      public:
        connect_to_nonexistent_port ( type::transition_type const&
                                    , type::connect_type const&
                                    );
      };

      // ******************************************************************* //

      class unknown_function : public generic
      {
      public:
        unknown_function (std::string const&, type::transition_type const&);

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

      class connect_eureka_to_nonexistent_out_port : public generic
      {
      public:
        connect_eureka_to_nonexistent_out_port
          ( type::transition_type const&
          , type::eureka_type const&
          );
      };

      class eureka_port_type_mismatch : public generic
      {
      public:
        eureka_port_type_mismatch
          ( type::transition_type const&
          , type::eureka_type const&
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
                           , type::connect_type const&
                           , type::port_type const&
                           , type::place_type const&
                           );
      };

      class memory_buffer_without_size : public generic
      {
      public:
        memory_buffer_without_size ( std::string const&
                                   , util::position_type const&
                                   );
        ~memory_buffer_without_size() noexcept override = default;
        memory_buffer_without_size (memory_buffer_without_size const&) = delete;
        memory_buffer_without_size (memory_buffer_without_size&&) = default;
        memory_buffer_without_size& operator= (memory_buffer_without_size const&) = delete;
        memory_buffer_without_size& operator= (memory_buffer_without_size&&) = delete;

      private:
        std::string const _name;
        util::position_type const _position_of_definition;
      };

      class memory_buffer_for_non_module : public generic
      {
      public:
        memory_buffer_for_non_module (type::function_type const&);
        ~memory_buffer_for_non_module() noexcept override = default;
        memory_buffer_for_non_module (memory_buffer_for_non_module const&) = delete;
        memory_buffer_for_non_module (memory_buffer_for_non_module&&) = default;
        memory_buffer_for_non_module& operator= (memory_buffer_for_non_module const&) = delete;
        memory_buffer_for_non_module& operator= (memory_buffer_for_non_module&&) = delete;
      };

      class memory_transfer_for_non_module : public generic
      {
      public:
        memory_transfer_for_non_module (type::function_type const&);
        ~memory_transfer_for_non_module() noexcept override = default;
        memory_transfer_for_non_module (memory_transfer_for_non_module const&) = delete;
        memory_transfer_for_non_module (memory_transfer_for_non_module&&) = default;
        memory_transfer_for_non_module& operator= (memory_transfer_for_non_module const&) = delete;
        memory_transfer_for_non_module& operator= (memory_transfer_for_non_module&&) = delete;
      };

      class memory_buffer_with_same_name_as_port : public generic
      {
      public:
        memory_buffer_with_same_name_as_port
          (type::memory_buffer_type const&, type::port_type const&);
        ~memory_buffer_with_same_name_as_port() noexcept override = default;
        memory_buffer_with_same_name_as_port (memory_buffer_with_same_name_as_port const&) = delete;
        memory_buffer_with_same_name_as_port (memory_buffer_with_same_name_as_port&&) = default;
        memory_buffer_with_same_name_as_port& operator= (memory_buffer_with_same_name_as_port const&) = delete;
        memory_buffer_with_same_name_as_port& operator= (memory_buffer_with_same_name_as_port&&) = delete;
      };

      // ******************************************************************* //

      class property_generic : public generic
      {
      public:
        property_generic ( std::string const& msg
                         , we::type::property::path_type const& key
                         , ::boost::filesystem::path const& path
                         )
          : generic
            { fmt::format ( "{} for property {}"
                          , msg
                          , fhg::util::join (key, '.')
                          )
            , path
            }
        {}
      };

      // ******************************************************************* //

      class type_map_mismatch : public generic
      {
      public:
        type_map_mismatch ( std::string const& from
                          , std::string const& to_old
                          , std::string const& to_new
                          , ::boost::filesystem::path const& path
                          )
          : generic
            { fmt::format ( "type map mismatch, type {} mapped to type {}"
                            " and earlier to type {}"
                          , from
                          , to_old
                          , to_new
                          )
            , path
            }
        {}
      };

      // ******************************************************************* //

      class missing_type_out : public generic
      {
      public:
        missing_type_out ( std::string const& type
                         , std::string const& spec
                         , ::boost::filesystem::path const& path
                         )
          : generic
            { fmt::format ( "missing type-out {} in specialization {}"
                          , type
                          , spec
                          )
            , path
            }
        {}
      };

      // ******************************************************************* //

      class invalid_prefix : public generic
      {
      public:
        invalid_prefix ( std::string const& name
                       , std::string const& type
                       , ::boost::filesystem::path const& path
                       )
          : generic
            { fmt::format ( "{1} {0} with invalid prefix"
                          , name
                          , type
                          )
            , path
            }
        {}
      };

      // ******************************************************************* //

      class invalid_name : public generic
      {
      public:
        invalid_name ( std::string const& name
                     , std::string const& type
                     , ::boost::filesystem::path const& path
                     )
          : generic
            { fmt::format
              ( "{1} {0} is invalid (not of the form: [a-zA-Z_][a-zA-Z_0-9]^*)"
              , name
              , type
              )
            , path
            }
        {}
      };

      // ******************************************************************* //

      class invalid_field_name : public generic
      {
      public:
        invalid_field_name ( std::string const& name
                           , ::boost::filesystem::path const& path
                           )
          : generic {fmt::format (" invalid field name {}", name), path}
        {}
      };

      // ******************************************************************* //

      class no_map_for_virtual_place : public generic
      {
      public:
        no_map_for_virtual_place ( std::string const& name
                                 , ::boost::filesystem::path const& path
                                 )
          : generic
            { fmt::format (" missing map for virtual place {}", name)
            , path
            }
        {}
      };

      // ******************************************************************* //

      class port_type_mismatch : public generic
      {
      public:
        port_type_mismatch ( type::port_type const& port
                           , type::port_type const& other_port
                           );
      };

      // ******************************************************************* //

      class real_place_missing : public generic
      {
      public:
        real_place_missing ( std::string const& place_virtual
                           , std::string const& place_real
                           , std::string const& trans
                           , ::boost::filesystem::path const& path
                           )
          : generic
            { fmt::format
              ( " missing real place {1} to replace virtual place {0}"
                " in transition {2}"
              , place_virtual
              , place_real
              , trans
              )
            , path
            }
        {}
      };

      // ******************************************************************* //

      namespace parse_function
      {
        class formatted : public generic
        {
        public:
          formatted ( std::string const& name
                    , std::string const& function
                    , std::string const& what
                    , std::size_t const& k
                    , ::boost::filesystem::path const& path
                    )
            : generic
              { fmt::format
                (R"EOS(error while parsing a function description for module name {0} in {4}:
{1}
{3}^
{2}
)EOS"
                , name
                , function
                , what
                , std::string (k, ' ')
                , path
                )
              }
          {}
        };

        class expected : public formatted
        {
        public:
          expected ( std::string const& name
                   , std::string const& function
                   , std::string const& what
                   , std::size_t const& k
                   , ::boost::filesystem::path const& path
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
        could_not_open_file (::boost::filesystem::path const& file)
          : could_not_open_file (file.string())
        {}

        could_not_open_file (std::string const& file)
          : generic {fmt::format ("could not open file {}", file)}
        {}
      };

      // ******************************************************************* //

      class could_not_create_directory : public generic
      {
      public:
        could_not_create_directory (::boost::filesystem::path const& path)
          : generic {fmt::format ("could not create directory {}", path)}
        {}
      };

      // ******************************************************************* //

      class template_without_function : public generic
      {
      public:
        template_without_function ( ::boost::optional<std::string> const& name
                                  , ::boost::filesystem::path const& path
                                  )
          : generic
            {fmt::format ("template {} without a function", name), path}
        {}
      };

      // ******************************************************************* //

      class weparse : public generic
      {
      public:
        weparse (std::string const& msg)
          : generic (msg)
        {}
      };

      // ******************************************************************* //

      class strange : public generic
      {
      public:
        strange (std::string const& msg)
          : generic ("this is STRANGE and should not happen", msg)
        {}
      };
    }
  }
}
