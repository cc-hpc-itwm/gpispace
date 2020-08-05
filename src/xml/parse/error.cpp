#include <xml/parse/error.hpp>

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/memory_buffer.hpp>
#include <xml/parse/type/net.hpp>
#include <xml/parse/type/place.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/port.hpp>
#include <xml/parse/type/response.hpp>
#include <xml/parse/type/eureka.hpp>
#include <xml/parse/type/template.hpp>
#include <xml/parse/type/transition.hpp>

#include <we/type/net.hpp>

#include <we/type/signature/show.hpp>
#include <we/workflow_response.hpp>

#include <util-generic/first_then.hpp>
#include <util-generic/print_container.hpp>

#include <boost/format.hpp>

namespace xml
{
  namespace parse
  {
    namespace error
    {
      wrong_node::wrong_node ( const rapidxml::node_type& want
                             , const rapidxml::node_type& got
                             , const util::position_type& position
                             )
        : generic ( boost::format ( "expected node of type %1%: got node of"
                                    " type %2% in %3%"
                                  )
                  % util::show_node_type (want)
                  % util::show_node_type (got)
                  % position
                  )
      { }

      wrong_node::wrong_node ( const rapidxml::node_type& want1
                             , const rapidxml::node_type& want2
                             , const rapidxml::node_type& got
                             , const util::position_type& position
                             )
        : generic ( boost::format ( "expected node of type %1%: or %2% got"
                                    " node of type %3% in %4%"
                                  )
                  % util::show_node_type (want1)
                  % util::show_node_type (want2)
                  % util::show_node_type (got)
                  % position
                  )
      { }

      missing_attr::missing_attr ( const std::string& pre
                                 , const std::string& attr
                                 , const util::position_type& position
                                 )
        : generic ( boost::format ("%1%: missing attribute %2% in %3%")
                  % pre
                  % attr
                  % position
                  )
      { }

      no_elements_given::no_elements_given ( const std::string& pre
                                           , const boost::filesystem::path& path
                                           )
        : generic ( boost::format ("%1%: no elements given at all in %2%")
                  % pre
                  % path
                  )
      { }

      more_than_one_definition::more_than_one_definition
        (const std::string& pre, const util::position_type& position)
          : generic ( boost::format ("%1%: more than one definition in %2%")
                    % pre
                    % position
                    )
      { }

      port_type_mismatch::port_type_mismatch
        ( const type::port_type& port
        , const type::port_type& other_port
        )
          : generic ( boost::format ( "in-/out-port %1% has different types: "
                                    "%2% (%3%) from %4% and %5% (%6%) from %7%"
                                    )
                    % port.name()
                    % port.type()
                    % we::type::enum_to_string (port.direction())
                    % port.position_of_definition()
                    % other_port.type()
                    % we::type::enum_to_string (other_port.direction())
                    % other_port.position_of_definition()
                    )
      { }

      port_not_connected::port_not_connected
        (const type::port_type& port, const boost::filesystem::path& path)
          : generic ( boost::format ("%1%-port %2% not connected in %3%")
                    % we::type::enum_to_string (port.direction())
                    % port.name()
                    % path
                    )
          , _path (path)
      { }

      port_connected_type_error::port_connected_type_error
        ( const type::port_type& port
        , const type::place_type& place
        , const boost::filesystem::path& path
        )
          : generic ( boost::format ( "type error: %1%-port %2% of type %3% "
                                      "connected to place %4% of type %5% in %6%"
                                    )
                    % we::type::enum_to_string (port.direction())
                    % port.name()
                    % port.type()
                    % place.name()
                    % place.type()
                    % path
                    )
        , _path (path)
      { }

      port_connected_place_nonexistent::port_connected_place_nonexistent
        ( const type::port_type& port
        , const boost::filesystem::path& path
        )
          : generic ( boost::format ( "%1%-port %2% connected to "
                                      "non-existing place %3% in %4%"
                                    )
                    % we::type::enum_to_string (port.direction())
                    % port.name()
                    % *port.place
                    % path
                    )
            , _path (path)
      { }

      tunnel_connected_non_virtual::tunnel_connected_non_virtual
        ( const type::port_type& port
        , const type::place_type& place
        , const boost::filesystem::path& path
        )
          : generic ( boost::format ( "tunnel %1% connected to non-virtual "
                                      "place %2% in %3%"
                                    )
                    % port.name()
                    % place.name()
                    % path
                    )
          , _path (path)
      { }

      tunnel_name_mismatch::tunnel_name_mismatch
        ( const type::port_type& port
        , const type::place_type& place
        , const boost::filesystem::path& path
        )
          : generic ( boost::format ( "tunnel %1% is connected to place with "
                                      "different name  %2% in %3%"
                                    )
                    % port.name()
                    % place.name()
                    % path
                    )
              , _path (path)
      { }

      unknown_function::unknown_function ( const std::string& fun
                                         , type::transition_type const& trans
                                         )
        : generic ( boost::format ( "unknown function %1% in transition %2%"
                                    " in %3%"
                                  )
                  % fun
                  % trans.name()
                  % trans.position_of_definition()
                  )
        , _function_name (fun)
      {}

      unknown_port_in_connect_response::unknown_port_in_connect_response
        (type::response_type const& response)
          : generic
            ( boost::format ("connect-response from unknown output port '%1%'")
            % response.port()
            , response.position_of_definition()
            )
      {}

      unknown_to_in_connect_response::unknown_to_in_connect_response
        (type::response_type const& response)
          : generic
            ( boost::format
              ("unknown input port '%1%' in attribute 'to' of connect-response")
            % response.to()
            , response.position_of_definition()
            )
      {}

      invalid_signature_in_connect_response::invalid_signature_in_connect_response
        ( type::response_type const& response
        , type::port_type const& port
        )
          : generic
            ( boost::format
              ("invalid signature for response to port '%1%'."
              " The type '%2%' with the signature '%3%' does not provide %4%"
              )
            % port.name()
            % port.type()
            % pnet::type::signature::show (port.signature())
            % we::response_description_requirements()
            , response.position_of_definition()
            )
      {}

      unknown_template::unknown_template ( type::specialize_type const& spec
                                         , type::net_type const& net
                                         )
        : generic ( boost::format ( "unknown template %1% in specialize at %2%"
                                    " in net at %3%"
                                  )
                  % spec.use
                  % spec.position_of_definition()
                  % net.position_of_definition()
                  )
      {}

      connect_to_nonexistent_place::connect_to_nonexistent_place
       ( type::transition_type const& transition
       , const type::connect_type& connection
       )
         : generic ( boost::format ( "connect-%1% to nonexistent place %2%"
                                     " in transition %3% at %4%"
                                   )
                   % connection.direction()
                   % connection.place()
                   % transition.name()
                   % connection.position_of_definition()
                   )
        {}

      connect_to_nonexistent_port::connect_to_nonexistent_port
       ( type::transition_type const& transition
       , const type::connect_type& connection
       )
         : generic ( boost::format ( "connect-%1% to nonexistent port %2%"
                                     " in transition %3% at %4%"
                                   )
                   % connection.direction()
                   % connection.port()
                   % transition.name()
                   % connection.position_of_definition()
                   )
        {}

      eureka_port_type_mismatch::eureka_port_type_mismatch
       ( type::transition_type const& transition
       , const type::eureka_type& eureka
       )
         : generic ( boost::format ( "connect-eureka output port %1%"
                                     " is not of type \"set\""
                                     " in transition %2% at %3%"
                                   )
                   % eureka.port()
                   % transition.name()
                   % eureka.position_of_definition()
                   )
        {}

      connect_eureka_to_nonexistent_out_port::connect_eureka_to_nonexistent_out_port
       ( type::transition_type const& transition
       , const type::eureka_type& eureka
       )
         : generic ( boost::format ( "connect-eureka to non-existent output port %1%"
                                     " in transition %2% at %3%"
                                   )
                   % eureka.port()
                   % transition.name()
                   % eureka.position_of_definition()
                   )
        {}

      connect_type_error::connect_type_error
        ( type::transition_type const& transition
        , const type::connect_type& connection
        , const type::port_type& port
        , const type::place_type& place
        )
         : generic ( boost::format ( "type-error: connect-%1%"
                                     " from place %2%::%3% (%4%) at %5%"
                                     " to port %6%::%7% (%8%) at %9%"
                                     " in transition %10% at %11%"
                                   )
                   % connection.direction()
                   % place.name()
                   % place.type()
                   % pnet::type::signature::show (place.signature())
                   % place.position_of_definition()
                   % port.name()
                   % port.type()
                   % pnet::type::signature::show (port.signature())
                   % port.position_of_definition()
                   % transition.name()
                   % connection.position_of_definition()
                   )
          {}

      memory_buffer_without_size::memory_buffer_without_size
        ( std::string const& name
        , util::position_type const& position_of_definition
        )
        : generic ( boost::format ("memory-buffer '%1%' without size, at %2%")
                  % name
                  % position_of_definition
                  )
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
          : generic ( boost::format
                      ( "non module call function '%1%'"
                      " with %2% memory buffer%3%"
                      ", function defined at %4%"
                      ", memory buffer%3% defined at %5%"
                      )
                    % function.name()
                    % function.memory_buffers().size()
                    % ((function.memory_buffers().size() > 1)
                       ? "s" : ""
                      )
                    % function.position_of_definition()
                    % print_memory_buffer_positions_of_definition
                      (function.memory_buffers())
                    )
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
          : generic ( boost::format
                      ( "non module call function '%1%'"
                      " with %2% memory transfer%3%"
                      ", function defined at %4%"
                      ", memory transfer%3% defined at:%5%"
                      )
                    % function.name()
                    % ( function.memory_gets().size()
                      + function.memory_puts().size()
                      + function.memory_getputs().size()
                      )
                    % ((( function.memory_gets().size()
                        + function.memory_puts().size()
                        + function.memory_getputs().size()
                        ) > 1
                       ) ? "s" : ""
                      )
                    % function.position_of_definition()
                    % print_memory_transfer_positions_of_definition (function)
                    )
      {}

      memory_buffer_with_same_name_as_port
        ::memory_buffer_with_same_name_as_port
        ( type::memory_buffer_type const& memory_buffer
        , type::port_type const& port
        )
          : generic ( boost::format
                      ("memory buffer '%1%' defined at %2%"
                      " with the same name as the %3%-port defined at %4%"
                      )
                    % memory_buffer.name()
                    % memory_buffer.position_of_definition()
                    % we::type::enum_to_string (port.direction())
                    % port.position_of_definition()
                    )
        {}

      cannot_resolve::cannot_resolve ( const std::string& field
                                     , const std::string& type
                                     , const type::structure_type& strct
                                     )
        : generic ( boost::format ("cannot resolve %1%::%2%, defined at %3%")
                  % field
                  % type
                  % strct.position_of_definition()
                  )
        , _field (field)
        , _type (type)
      {}

      struct_redefined::struct_redefined ( const type::structure_type& early
                                         , const type::structure_type& late
                                         )
        : generic ( boost::format ("struct %1% at %2% redefined at %3%")
                  % early.name()
                  % early.position_of_definition()
                  % late.position_of_definition()
                  )
      {}

      duplicate_specialize::duplicate_specialize
        ( type::specialize_type const& early
        , type::specialize_type const& late
        )
          : generic_duplicate<type::specialize_type>
            (early, late, boost::format ("specialize %1%") % early.name())
      {}

      duplicate_place::duplicate_place ( const type::place_type& early
                                       , const type::place_type& late
                                       )
        : generic_duplicate<type::place_type>
          (early, late, boost::format ("place %1%") % early.name())
      {}

      duplicate_transition::duplicate_transition
        ( type::transition_type const& early
        , type::transition_type const& late
        )
          : generic_duplicate<type::transition_type>
            (early, late, boost::format ("transition %1%") % early.name())
      {}

      duplicate_port::duplicate_port ( const type::port_type& early
                                     , const type::port_type& late
                                     )
        : generic_duplicate<type::port_type>
            ( early
            , late
            , boost::format ("%1%-port %2%")
            % we::type::enum_to_string (early.direction())
            % early.name()
            )
      {}

      duplicate_template::duplicate_template
        ( const type::tmpl_type& early
        , const type::tmpl_type& late
        )
          : generic_duplicate<type::tmpl_type>
            (early, late, boost::format ("template %1%") % early.name())
      {}

      duplicate_place_map::duplicate_place_map
        ( const type::place_map_type& early
        , const type::place_map_type& late
        )
          : generic_duplicate<type::place_map_type>
              ( early
              , late
              , boost::format ("place-map %1% <-> %2%")
              % early.place_virtual()
              % early.place_real()
              )
      {}

      duplicate_external_function::duplicate_external_function
         ( const type::module_type& early
         , const type::module_type& late
         )
          : generic_duplicate<type::module_type>
            ( early
            , late
            , boost::format ( "external function %1% in module %2%"
                              " has conflicting definition"
                            )
            % early.function()
            % early.name()
            )
      {}

      duplicate_connect::duplicate_connect
        ( const type::connect_type& early
        , const type::connect_type& late
        )
          : generic_duplicate<type::connect_type>
            ( early
            , late
            , boost::format ( "connect-%1% %2% <-> %3%"
                              " (existing connection is connect-%4%)"
                            )
            % we::edge::enum_to_string (late.direction())
            % late.place()
            % late.port()
            % we::edge::enum_to_string (early.direction())
            )
      {}

      duplicate_response::duplicate_response
        ( const type::response_type& early
        , const type::response_type& late
        )
          : generic_duplicate<type::response_type>
            ( early
            , late
            , boost::format ( "connect-response %1% -> %2%"
                              " (existing response connects to %3%)"
                            )
            % late.port()
            % late.to()
            % early.to()
            )
      {}

      duplicate_eureka::duplicate_eureka
        ( const type::eureka_type& early
        , const type::eureka_type& late
        )
          : generic_duplicate<type::eureka_type>
            ( early
            , late
            , boost::format ( "duplicate connect-eureka"
                              " for port: %1%"
                            )
            % early.port()
            )
      {}

      duplicate_memory_buffer::duplicate_memory_buffer
        ( type::memory_buffer_type const& early
        , type::memory_buffer_type const& late
        )
          : generic_duplicate<type::memory_buffer_type>
            ( early
            , late
            , boost::format ("memory-buffer '%1%'") % late.name()
            )
      {}

      place_type_unknown::place_type_unknown (const type::place_type& place)
        : generic ( boost::format ("unknown type %1% for place %2% at %3%")
                  % place.type()
                  % place.name()
                  % place.position_of_definition()
                  )
      {}

      duplicate_preference::duplicate_preference
        (const std::string& target, const util::position_type& position)
          : generic ( boost::format ( "duplicate target type '%1%' at %2%"
                                      ", already in the preferences"
                                    )
                    % target
                    % position
                    )
      {}

      empty_preferences::empty_preferences
        (const util::position_type& position)
          : generic ( boost::format ( "preferences enabled, but no targets"
                                      " specified at %1%"
                                    )
                    % position
                    )
      {}

      preferences_without_modules::preferences_without_modules
        (const util::position_type& position)
          : generic ( boost::format ( "preferences enabled, but no modules"
                                      " with target defined in %1%"
                                    )
                    % position
                    )
      {}

      missing_target_for_module::missing_target_for_module
        (const std::string& module, const util::position_type& position)
          : generic ( boost::format ( "module '%1%' missing target"
                                      " for multi-module transition at %2%"
                                    )
                    % module
                    % position
                    )
      {}

      modules_without_preferences::modules_without_preferences
        ( const std::string& module
        , const std::string& target
        , const util::position_type& position
        )
          : generic ( boost::format ( "module '%1%' defined with target '%2%'"
                                      ", but preferences not enabled at %3%"
                                    )
                    % module
                    % target
                    % position
                    )
      {}

      duplicate_module_for_target::duplicate_module_for_target
        ( const std::string& module
        , const std::string& target
        , const util::position_type& position
        )
          : generic ( boost::format ( "duplicate module '%1%' for target '%2%'"
                                      " at %3%"
                                    )
                    % module
                    % target
                    % position
                    )
      {}

      mismatching_eureka_for_module::mismatching_eureka_for_module
        (const std::string& module, const util::position_type& position)
          : generic ( boost::format ( "mismatching eureka group for module '%1%'"
                                      " in multi-module transition at %2%"
                                    )
                    % module
                    % position
                    )
      {}

      namespace
      {
        std::string print_target_list ( const std::string& _prefix
                               , const std::list<std::string>& _list
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
        ( const std::list<std::string>& missing_in_preferences
        , const std::list<std::string>& missing_in_modules
        , const util::position_type& position
        )
          : generic ( boost::format ( "mismatching targets for multi-module"
                                      " transition in %3%, %1%%2%"
                                    )
                    % print_target_list ( "mismatch-in-preferences"
                                        , missing_in_preferences
                                        )
                    % print_target_list ( ", mismatch-in-modules"
                                        , missing_in_modules
                                        )
                    % position
                    )
      {}
    }
  }
}
