// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/error.hpp>

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/eureka.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/memory_buffer.hpp>
#include <xml/parse/type/net.hpp>
#include <xml/parse/type/place.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/port.hpp>
#include <xml/parse/type/response.hpp>
#include <xml/parse/type/template.hpp>
#include <xml/parse/type/transition.hpp>

#include <we/type/net.hpp>

#include <we/type/signature/show.hpp>
#include <we/workflow_response.hpp>

#include <util-generic/first_then.hpp>
#include <util-generic/print_container.hpp>

#include <FMT/we/type/net.hpp>
#include <fmt/core.h>

namespace xml
{
  namespace parse
  {
    namespace error
    {
      wrong_node::wrong_node ( rapidxml::node_type const& want
                             , rapidxml::node_type const& got
                             , util::position_type const& position
                             )
        : generic { fmt::format ( "expected node of type {}: got node of"
                                  " type {} in {}"
                                , util::show_node_type (want)
                                , util::show_node_type (got)
                                , position
                                )
                  }
      { }

      wrong_node::wrong_node ( rapidxml::node_type const& want1
                             , rapidxml::node_type const& want2
                             , rapidxml::node_type const& got
                             , util::position_type const& position
                             )
        : generic { fmt::format ( "expected node of type {}: or {} got"
                                  " node of type {} in {}"
                                , util::show_node_type (want1)
                                , util::show_node_type (want2)
                                , util::show_node_type (got)
                                , position
                                )
                  }
      { }

      missing_attr::missing_attr ( std::string const& pre
                                 , std::string const& attr
                                 , util::position_type const& position
                                 )
        : generic { fmt::format ( "{}: missing attribute {} in {}"
                                , pre
                                , attr
                                , position
                                )
                  }
      { }

      no_elements_given::no_elements_given ( std::string const& pre
                                           , ::boost::filesystem::path const& path
                                           )
        : generic { fmt::format ( "{}: no elements given at all in {}"
                                , pre
                                , path
                                )
                  }
      { }

      more_than_one_definition::more_than_one_definition
        (std::string const& pre, util::position_type const& position)
          : generic { fmt::format ( "{}: more than one definition in {}"
                                  , pre
                                  , position
                                  )
                    }
      { }

      port_type_mismatch::port_type_mismatch
        ( type::port_type const& port
        , type::port_type const& other_port
        )
          : generic { fmt::format ( "in-/out-port {} has different types: "
                                    "{} ({}) from {} and {} ({}) from {}"
                                  , port.name()
                                  , port.type()
                                  , port.direction()
                                  , port.position_of_definition()
                                  , other_port.type()
                                  , other_port.direction()
                                  , other_port.position_of_definition()
                                  )
                    }
      { }

      port_not_connected::port_not_connected
        (type::port_type const& port, ::boost::filesystem::path const& path)
          : generic { fmt::format ( "{}-port {} not connected in {}"
                                  , port.direction()
                                  , port.name()
                                  , path
                                  )
                    }
          , _path (path)
      { }

      port_connected_type_error::port_connected_type_error
        ( type::port_type const& port
        , type::place_type const& place
        , ::boost::filesystem::path const& path
        )
          : generic { fmt::format ( "type error: {}-port {} of type {} "
                                    "connected to place {} of type {} in {}"
                                  , port.direction()
                                  , port.name()
                                  , port.type()
                                  , place.name()
                                  , place.type()
                                  , path
                                  )
                    }
        , _path (path)
      { }

      port_connected_place_nonexistent::port_connected_place_nonexistent
        ( type::port_type const& port
        , ::boost::filesystem::path const& path
        )
          : generic { fmt::format ( "{}-port {} connected to "
                                    "non-existing place {} in {}"
                                  , port.direction()
                                  , port.name()
                                  , *port.place
                                  , path
                                  )
                    }
            , _path (path)
      { }

      tunnel_connected_non_virtual::tunnel_connected_non_virtual
        ( type::port_type const& port
        , type::place_type const& place
        , ::boost::filesystem::path const& path
        )
          : generic { fmt::format ( "tunnel {} connected to non-virtual "
                                    "place {} in {}"
                                  , port.name()
                                  , place.name()
                                  , path
                                  )
                    }
          , _path (path)
      { }

      tunnel_name_mismatch::tunnel_name_mismatch
        ( type::port_type const& port
        , type::place_type const& place
        , ::boost::filesystem::path const& path
        )
          : generic { fmt::format ( "tunnel {} is connected to place with "
                                    "different name {} in {}"
                                  , port.name()
                                  , place.name()
                                  , path
                                  )
                    }
              , _path (path)
      { }

      unknown_function::unknown_function ( std::string const& fun
                                         , type::transition_type const& trans
                                         )
        : generic { fmt::format ( "unknown function {} in transition {}"
                                  " in {}"
                                , fun
                                , trans.name()
                                , trans.position_of_definition()
                                )
                  }
        , _function_name (fun)
      {}

      unknown_port_in_connect_response::unknown_port_in_connect_response
        (type::response_type const& response)
          : generic
            { fmt::format ( "connect-response from unknown output port '{}'"
                          , response.port()
                          )
            , response.position_of_definition()
            }
      {}

      unknown_to_in_connect_response::unknown_to_in_connect_response
        (type::response_type const& response)
          : generic
            { fmt::format
              ( "unknown input port '{}' in attribute 'to' of connect-response"
              , response.to()
              )
            , response.position_of_definition()
            }
      {}

      invalid_signature_in_connect_response::invalid_signature_in_connect_response
        ( type::response_type const& response
        , type::port_type const& port
        )
          : generic
            { fmt::format
              ( "invalid signature for response to port '{}'."
                " The type '{}' with the signature '{}' does not provide {}"
              , port.name()
              , port.type()
              , pnet::type::signature::show (port.signature())
              , we::response_description_requirements()
              )
            , response.position_of_definition()
            }
      {}

      unknown_template::unknown_template ( type::specialize_type const& spec
                                         , type::net_type const& net
                                         )
        : generic { fmt::format ( "unknown template {} in specialize at {}"
                                  " in net at {}"
                                , spec.use
                                , spec.position_of_definition()
                                , net.position_of_definition()
                                )
                 }
      {}

      connect_to_nonexistent_place::connect_to_nonexistent_place
       ( type::transition_type const& transition
       , type::connect_type const& connection
       )
         : generic { fmt::format ( "connect-{} to nonexistent place {}"
                                   " in transition {} at {}"
                                 , connection.direction()
                                 , connection.place()
                                 , transition.name()
                                 , connection.position_of_definition()
                                 )
                   }
        {}

      connect_to_nonexistent_port::connect_to_nonexistent_port
       ( type::transition_type const& transition
       , type::connect_type const& connection
       )
         : generic { fmt::format ( "connect-{} to nonexistent port {}"
                                   " in transition {} at {}"
                                 , connection.direction()
                                 , connection.port()
                                 , transition.name()
                                 , connection.position_of_definition()
                                 )
                   }
        {}

      eureka_port_type_mismatch::eureka_port_type_mismatch
       ( type::transition_type const& transition
       , type::eureka_type const& eureka
       )
         : generic { fmt::format ( "connect-eureka output port {}"
                                   " is not of type \"set\""
                                   " in transition {} at {}"
                                 , eureka.port()
                                 , transition.name()
                                 , eureka.position_of_definition()
                                 )
                   }
        {}

      eureka_group_attribute_and_tag::eureka_group_attribute_and_tag
          ( std::string const& module_name
          , we::type::eureka_id_type const& id_attribute
          , util::position_type const& pod_attribute
          , we::type::eureka_id_type const& id_tag
          , util::position_type const& pod_tag
          )
            : generic
              { fmt::format
                 ( "both are given:"
                   " the eureka attribute '{1}' at '{2}'"
                   " and the eureka tag '{3}' at '{4}'"
                   " in module '{0}'"
                   " Define only the attribute or only the tag."
                 , module_name
                 , id_attribute
                 , pod_attribute
                 , id_tag
                 , pod_tag
                 )
              }
      {}

      connect_eureka_to_nonexistent_out_port::connect_eureka_to_nonexistent_out_port
       ( type::transition_type const& transition
       , type::eureka_type const& eureka
       )
         : generic { fmt::format ( "connect-eureka to non-existent output port {}"
                                   " in transition {} at {}"
                                 , eureka.port()
                                 , transition.name()
                                 , eureka.position_of_definition()
                                 )
                   }
        {}

      connect_type_error::connect_type_error
        ( type::transition_type const& transition
        , type::connect_type const& connection
        , type::port_type const& port
        , type::place_type const& place
        )
          : generic { fmt::format ( "type-error: connect-{}"
                                    " from place {}::{} ({}) at {}"
                                    " to port {}::{} ({}) at {}"
                                    " in transition ,10, at ,11,"
                                  , connection.direction()
                                  , place.name()
                                  , place.type()
                                  , pnet::type::signature::show (place.signature())
                                  , place.position_of_definition()
                                  , port.name()
                                  , port.type()
                                  , pnet::type::signature::show (port.signature())
                                  , port.position_of_definition()
                                  , transition.name()
                                  , connection.position_of_definition()
                                  )
                    }
          {}

      memory_buffer_without_size::memory_buffer_without_size
        ( std::string const& name
        , util::position_type const& position_of_definition
        )
        : generic { fmt::format ( "memory-buffer '{}' without size, at {}"
                                , name
                                , position_of_definition
                                )
                  }
        , _name (name)
        , _position_of_definition (position_of_definition)
      {}

      namespace
      {
        template<typename MemoryBuffers>
        std::string print_memory_buffer_positions_of_definition
          (MemoryBuffers const& memory_buffers)
        {
          return fhg::util::print_container
            ( "{", ", ", "}", memory_buffers
            , fhg::util::ostream::callback::generic< type::memory_buffer_type
                                                   , util::position_type
                                                   >
              ([] (type::memory_buffer_type const& memory_buffer)
               {
                 return memory_buffer.position_of_definition();
               }
              )
            ).string();
        }
      }

      memory_buffer_for_non_module::memory_buffer_for_non_module
        (type::function_type const& function)
          : generic { fmt::format
                      ( "non module call function '{0}'"
                        " with {1} memory buffer{2}"
                        ", function defined at {3}"
                        ", memory buffer{2} defined at {4}"
                      , function.name()
                      , function.memory_buffers().size()
                      , (function.memory_buffers().size() > 1) ? "s" : ""
                      , function.position_of_definition()
                      , print_memory_buffer_positions_of_definition
                        (function.memory_buffers())
                      )
                    }
      {}

      namespace
      {
        template<typename T>
        fhg::util::join_reference<std::list<T>, std::string>
          print_positions (std::string const& prefix, std::list<T> const& list)
        {
          return fhg::util::print_container
              ( prefix + ": (", ", ", ")", list
              , fhg::util::ostream::callback::select<T, util::position_type>
                  (&T::position_of_definition)
              );
        }

        std::string print_memory_transfer_positions_of_definition
          (type::function_type const& function)
        {
          fhg::util::first_then<std::string> sep (" ", ", ");

          std::ostringstream oss;

          if (!function.memory_gets().empty())
          {
            oss << sep << print_positions<xml::parse::type::memory_get>
                            ("get", function.memory_gets())
              ;
          }

          if (!function.memory_puts().empty())
          {
            oss << sep << print_positions<xml::parse::type::memory_put>
                            ("put", function.memory_puts())
              ;
          }

          if (!function.memory_getputs().empty())
          {
            oss << sep << print_positions<xml::parse::type::memory_getput>
                            ("getput", function.memory_getputs())
              ;
          }

          return oss.str();
        }
      }

      memory_transfer_for_non_module::memory_transfer_for_non_module
        (type::function_type const& function)
          : generic { fmt::format
                      ( "non module call function '{0}'"
                        " with {1} memory transfer{2}"
                        ", function defined at {3}"
                        ", memory transfer{2} defined at:{4}"
                      , function.name()
                      , ( function.memory_gets().size()
                        + function.memory_puts().size()
                        + function.memory_getputs().size()
                        )
                      , ((( function.memory_gets().size()
                          + function.memory_puts().size()
                          + function.memory_getputs().size()
                          ) > 1
                         ) ? "s" : ""
                        )
                      , function.position_of_definition()
                      , print_memory_transfer_positions_of_definition (function)
                      )
                    }
      {}

      memory_buffer_with_same_name_as_port
        ::memory_buffer_with_same_name_as_port
        ( type::memory_buffer_type const& memory_buffer
        , type::port_type const& port
        )
          : generic { fmt::format
                      ( "memory buffer '{}' defined at {}"
                        " with the same name as the {}-port defined at {}"
                      , memory_buffer.name()
                      , memory_buffer.position_of_definition()
                      , port.direction()
                      , port.position_of_definition()
                      )
                    }
        {}

      cannot_resolve::cannot_resolve ( std::string const& field
                                     , std::string const& type
                                     , type::structure_type const& strct
                                     )
        : generic { fmt::format ( "cannot resolve {}::{}, defined at {}"
                                , field
                                , type
                                , strct.position_of_definition()
                                )
                  }
        , _field (field)
        , _type (type)
      {}

      struct_redefined::struct_redefined ( type::structure_type const& early
                                         , type::structure_type const& late
                                         )
        : generic { fmt::format ( "struct {} at {} redefined at {}"
                                , early.name()
                                , early.position_of_definition()
                                , late.position_of_definition()
                                )
                  }
      {}

      duplicate_specialize::duplicate_specialize
        ( type::specialize_type const& early
        , type::specialize_type const& late
        )
          : generic_duplicate<type::specialize_type>
            (early, late, fmt::format ("specialize {}", early.name()))
      {}

      duplicate_place::duplicate_place ( type::place_type const& early
                                       , type::place_type const& late
                                       )
        : generic_duplicate<type::place_type>
          (early, late, fmt::format ("place {}", early.name()))
      {}

      duplicate_transition::duplicate_transition
        ( type::transition_type const& early
        , type::transition_type const& late
        )
          : generic_duplicate<type::transition_type>
            (early, late, fmt::format ("transition {}", early.name()))
      {}

      duplicate_port::duplicate_port ( type::port_type const& early
                                     , type::port_type const& late
                                     )
        : generic_duplicate<type::port_type>
            ( early
            , late
            , fmt::format ( "{}-port {}"
                          , early.direction()
                          , early.name()
                          )
            )
      {}

      duplicate_template::duplicate_template
        ( type::tmpl_type const& early
        , type::tmpl_type const& late
        )
          : generic_duplicate<type::tmpl_type>
            (early, late, fmt::format ("template {}", early.name()))
      {}

      duplicate_place_map::duplicate_place_map
        ( type::place_map_type const& early
        , type::place_map_type const& late
        )
          : generic_duplicate<type::place_map_type>
              ( early
              , late
              , fmt::format ( "place-map {} <-> {}"
                            , early.place_virtual()
                            , early.place_real()
                            )
              )
      {}

      duplicate_external_function::duplicate_external_function
         ( type::module_type const& early
         , type::module_type const& late
         )
          : generic_duplicate<type::module_type>
            ( early
            , late
            , fmt::format ( "external function {} in module {}"
                            " has conflicting definition"
                          , early.function()
                          , early.name()
                          )
            )
      {}

      duplicate_connect::duplicate_connect
        ( type::connect_type const& early
        , type::connect_type const& late
        )
          : generic_duplicate<type::connect_type>
            ( early
            , late
            , fmt::format ( "connect-{} {} <-> {}"
                            " (existing connection is connect-{})"
                          , late.direction()
                          , late.place()
                          , late.port()
                          , early.direction()
                          )
            )
      {}

      duplicate_response::duplicate_response
        ( type::response_type const& early
        , type::response_type const& late
        )
          : generic_duplicate<type::response_type>
            ( early
            , late
            , fmt::format ( "connect-response {} -> {}"
                            " (existing response connects to {})"
                          , late.port()
                          , late.to()
                          , early.to()
                          )
            )
      {}

      duplicate_eureka::duplicate_eureka
        ( type::eureka_type const& early
        , type::eureka_type const& late
        )
          : generic_duplicate<type::eureka_type>
            ( early
            , late
            , fmt::format ( "duplicate connect-eureka for port: {}"
                          , early.port()
                          )
            )
      {}

      duplicate_memory_buffer::duplicate_memory_buffer
        ( type::memory_buffer_type const& early
        , type::memory_buffer_type const& late
        )
          : generic_duplicate<type::memory_buffer_type>
            ( early
            , late
            , fmt::format ("memory-buffer '{}'", late.name())
            )
      {}

      place_type_unknown::place_type_unknown (type::place_type const& place)
        : generic { fmt::format ( "unknown type {} for place {} at {}"
                                , place.type()
                                , place.name()
                                , place.position_of_definition()
                                )
                  }
      {}

      duplicate_preference::duplicate_preference
        (std::string const& target, util::position_type const& position)
          : generic { fmt::format ( "duplicate target type '{}' at {}"
                                      ", already in the preferences"
                                  , target
                                  , position
                                  )
                    }
      {}

      empty_preferences::empty_preferences
        (util::position_type const& position)
          : generic { fmt::format ( "preferences enabled, but no targets"
                                    " specified at {}"
                                  , position
                                  )
                    }
      {}

      preferences_without_modules::preferences_without_modules
        (util::position_type const& position)
           : generic { fmt::format ( "preferences enabled, but no modules"
                                     " with target defined in {}"
                                   , position
                                   )
                     }
      {}

      missing_target_for_module::missing_target_for_module
        (std::string const& module, util::position_type const& position)
          : generic { fmt::format ( "module '{}' missing target"
                                    " for multi-module transition at {}"
                                  , module
                                  , position
                                  )
                    }
      {}

      modules_without_preferences::modules_without_preferences
        ( std::string const& module
        , std::string const& target
        , util::position_type const& position
        )
           : generic { fmt::format ( "module '{}' defined with target '{}'"
                                     ", but preferences not enabled at {}"
                                   , module
                                   , target
                                   , position
                                   )
                     }
      {}

      modules_without_preferences::modules_without_preferences
          (util::position_type const& position)
        : generic { fmt::format ( "modules without preferences at {}"
                                , position
                                )
                  }
      {}

      duplicate_module_for_target::duplicate_module_for_target
        ( std::string const& module
        , std::string const& target
        , util::position_type const& position
        )
          : generic { fmt::format ( "duplicate module '{}' for target '{}'"
                                    " at {}"
                                  , module
                                  , target
                                  , position
                                  )
                    }
      {}

      mismatching_eureka_for_module::mismatching_eureka_for_module
        (std::string const& module, util::position_type const& position)
          : generic { fmt::format ( "mismatching eureka group for module '{}'"
                                    " in multi-module transition at {}"
                                  , module
                                  , position
                                  )
                    }
      {}

      namespace
      {
        std::string print_target_list ( std::string const& _prefix
                               , std::list<std::string> const& _list
                               )
       {
         return !_list.empty() ? fhg::util::print_container ( _prefix + " ('"
                                                            , "', '"
                                                            , "')"
                                                            , _list
                                                            ).string()
                               : "";
       }
      }

      mismatching_modules_and_preferences::mismatching_modules_and_preferences
        ( std::list<std::string> const& missing_in_preferences
        , std::list<std::string> const& missing_in_modules
        , util::position_type const& position
        )
          : generic { fmt::format ( "mismatching targets for multi-module"
                                    " transition in {2}, {0}{1}"
                                  , print_target_list ( "mismatch-in-preferences"
                                                      , missing_in_preferences
                                                      )
                                  , print_target_list ( ", mismatch-in-modules"
                                                      , missing_in_modules
                                                      )
                                  , position
                                  )
                    }
      {}
    }
  }
}
