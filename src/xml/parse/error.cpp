// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/error.hpp>

#include <xml/parse/type/connect.hpp>
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
        ( const id::ref::port& port
        , const id::ref::port& other_port
        )
          : generic ( boost::format ( "in-/out-port %1% has different types: "
                                    "%2% (%3%) from %4% and %5% (%6%) from %7%"
                                    )
                    % port.get().name()
                    % port.get().type()
                    % we::type::enum_to_string (port.get().direction())
                    % port.get().position_of_definition()
                    % other_port.get().type()
                    % we::type::enum_to_string (other_port.get().direction())
                    % other_port.get().position_of_definition()
                    )
        , _port (port)
        , _other_port (other_port)
      { }

      port_not_connected::port_not_connected
        (const id::ref::port& port, const boost::filesystem::path& path)
          : generic ( boost::format ("%1%-port %2% not connected in %3%")
                    % we::type::enum_to_string (port.get().direction())
                    % port.get().name()
                    % path
                    )
          , _port (port)
          , _path (path)
      { }

      port_connected_type_error::port_connected_type_error
        ( const id::ref::port& port
        , const id::ref::place& place
        , const boost::filesystem::path& path
        )
          : generic ( boost::format ( "type error: %1%-port %2% of type %3% "
                                      "connected to place %4% of type %5% in %6%"
                                    )
                    % we::type::enum_to_string (port.get().direction())
                    % port.get().name()
                    % port.get().type()
                    % place.get().name()
                    % place.get().type()
                    % path
                    )
        , _port (port)
        , _place (place)
        , _path (path)
      { }

      port_connected_place_nonexistent::port_connected_place_nonexistent
        ( const id::ref::port& port
        , const boost::filesystem::path& path
        )
          : generic ( boost::format ( "%1%-port %2% connected to "
                                      "non-existing place %3% in %4%"
                                    )
                    % we::type::enum_to_string (port.get().direction())
                    % port.get().name()
                    % *port.get().place
                    % path
                    )
          , _port (port)
          , _path (path)
      { }

      tunnel_connected_non_virtual::tunnel_connected_non_virtual
        ( const id::ref::port& port
        , const id::ref::place& place
        , const boost::filesystem::path& path
        )
          : generic ( boost::format ( "tunnel %1% connected to non-virtual "
                                      "place %2% in %3%"
                                    )
                    % port.get().name()
                    % place.get().name()
                    % path
                    )
          , _port (port)
          , _place (place)
          , _path (path)
      { }

      tunnel_name_mismatch::tunnel_name_mismatch
        ( const id::ref::port& port
        , const id::ref::place& place
        , const boost::filesystem::path& path
        )
          : generic ( boost::format ( "tunnel %1% is connected to place with "
                                      "different name  %2% in %3%"
                                    )
                    % port.get().name()
                    % place.get().name()
                    % path
                    )
          , _port (port)
          , _place (place)
          , _path (path)
      { }

      unknown_function::unknown_function ( const std::string& fun
                                         , const id::ref::transition& trans
                                         )
        : generic ( boost::format ( "unknown function %1% in transition %2%"
                                    " in %3%"
                                  )
                  % fun
                  % trans.get().name()
                  % trans.get().position_of_definition()
                  )
        , _function_name (fun)
        , _transition (trans)
      {}

      unknown_port_in_connect_response::unknown_port_in_connect_response
        (id::ref::response const& response)
          : generic
            ( boost::format ("connect-response from unknown output port '%1%'")
            % response.get().port()
            , response.get().position_of_definition()
            )
      {}

      unknown_to_in_connect_response::unknown_to_in_connect_response
        (id::ref::response const& response)
          : generic
            ( boost::format
              ("unknown input port '%1%' in attribute 'to' of connect-response")
            % response.get().to()
            , response.get().position_of_definition()
            )
      {}

      invalid_signature_in_connect_response::invalid_signature_in_connect_response
        ( id::ref::response const& response
        , id::ref::port const& port
        )
          : generic
            ( boost::format
              ("invalid signature for response to port '%1%'."
              " The type '%2%' with the signature '%3%' does not provide %4%"
              )
            % port.get().name()
            % port.get().type()
            % pnet::type::signature::show (port.get().signature_or_throw())
            % we::rpc_server_description_requirements()
            , response.get().position_of_definition()
            )
      {}

      unknown_template::unknown_template ( const id::ref::specialize& spec
                                         , const id::ref::net& net
                                         )
        : generic ( boost::format ( "unknown template %1% in specialize at %2%"
                                    " in net at %3%"
                                  )
                  % spec.get().use
                  % spec.get().position_of_definition()
                  % net.get().position_of_definition()
                  )
        , _specialize (spec)
        , _net (net)
      {}

      connect_to_nonexistent_place::connect_to_nonexistent_place
       ( const id::ref::transition& transition
       , const id::ref::connect& connection
       )
         : generic ( boost::format ( "connect-%1% to nonexistent place %2%"
                                     " in transition %3% at %4%"
                                   )
                   % connection.get().direction()
                   % connection.get().place()
                   % transition.get().name()
                   % connection.get().position_of_definition()
                   )
         , _transition (transition)
         , _connection (connection)
        {}

      connect_to_nonexistent_port::connect_to_nonexistent_port
       ( const id::ref::transition& transition
       , const id::ref::connect& connection
       )
         : generic ( boost::format ( "connect-%1% to nonexistent port %2%"
                                     " in transition %3% at %4%"
                                   )
                   % connection.get().direction()
                   % connection.get().port()
                   % transition.get().name()
                   % connection.get().position_of_definition()
                   )
         , _transition (transition)
         , _connection (connection)
        {}

      connect_type_error::connect_type_error
        ( const id::ref::transition& transition
        , const id::ref::connect& connection
        , const id::ref::port& port
        , const id::ref::place& place
        )
         : generic ( boost::format ( "type-error: connect-%1%"
                                     " from place %2%::%3% (%4%) at %5%"
                                     " to port %6%::%7% (%8%) at %9%"
                                     " in transition %10% at %11%"
                                   )
                   % connection.get().direction()
                   % place.get().name()
                   % place.get().type()
                   % pnet::type::signature::show (place.get().signature_or_throw())
                   % place.get().position_of_definition()
                   % port.get().name()
                   % port.get().type()
                   % pnet::type::signature::show (port.get().signature_or_throw())
                   % port.get().position_of_definition()
                   % transition.get().name()
                   % connection.get().position_of_definition()
                   )
          , _transition (transition)
          , _connection (connection)
          , _port (port)
          , _place (place)
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
        std::string print_memory_buffer_positions_of_definition
          (std::unordered_set<id::ref::memory_buffer> const& ids)
        {
          return fhg::util::print_container
            ( "{", ", ", "}", ids
            , fhg::util::ostream::callback::generic< id::ref::memory_buffer
                                                   , util::position_type
                                                   >
              ([] (id::ref::memory_buffer const& id)
               {
                 return id.get().position_of_definition();
               }
              )
            ).string();
        }
      }

      memory_buffer_for_non_module::memory_buffer_for_non_module
        (id::ref::function const& function)
          : generic ( boost::format
                      ( "non module call function '%1%'"
                      " with %2% memory buffer%3%"
                      ", function defined at %4%"
                      ", memory buffer%3% defined at %5%"
                      )
                    % function.get().name()
                    % function.get().memory_buffers().ids().size()
                    % ((function.get().memory_buffers().ids().size() > 1)
                       ? "s" : ""
                      )
                    % function.get().position_of_definition()
                    % print_memory_buffer_positions_of_definition
                      (function.get().memory_buffers().ids())
                    )
          , _function (function)
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
        };

        std::string print_memory_transfer_positions_of_definition
          (id::ref::function const& function)
        {
          fhg::util::first_then<std::string> sep (" ", ", ");

          std::ostringstream oss;

          if (!function.get_ref().memory_gets().empty())
          {
            oss << sep << print_positions<xml::parse::type::memory_get>
                            ("get", function.get_ref().memory_gets())
              ;
          }

          if (!function.get_ref().memory_puts().empty())
          {
            oss << sep << print_positions<xml::parse::type::memory_put>
                            ("put", function.get_ref().memory_puts())
              ;
          }

          if (!function.get_ref().memory_getputs().empty())
          {
            oss << sep << print_positions<xml::parse::type::memory_getput>
                            ("getput", function.get_ref().memory_getputs())
              ;
          }

          return oss.str();
        }
      }

      memory_transfer_for_non_module::memory_transfer_for_non_module
        (id::ref::function const& function)
          : generic ( boost::format
                      ( "non module call function '%1%'"
                      " with %2% memory transfer%3%"
                      ", function defined at %4%"
                      ", memory transfer%3% defined at:%5%"
                      )
                    % function.get().name()
                    % ( function.get().memory_gets().size()
                      + function.get().memory_puts().size()
                      + function.get().memory_getputs().size()
                      )
                    % ((( function.get().memory_gets().size()
                        + function.get().memory_puts().size()
                        + function.get().memory_getputs().size()
                        ) > 1
                       ) ? "s" : ""
                      )
                    % function.get().position_of_definition()
                    % print_memory_transfer_positions_of_definition (function)
                    )
        , _function (function)
      {}

      memory_buffer_with_same_name_as_port
        ::memory_buffer_with_same_name_as_port
        ( id::ref::memory_buffer const& memory_buffer
        , id::ref::port const& port
        )
          : generic ( boost::format
                      ("memory buffer '%1%' defined at %2%"
                      " with the same name as the %3%-port defined at %4%"
                      )
                    % memory_buffer.get().name()
                    % memory_buffer.get().position_of_definition()
                    % we::type::enum_to_string (port.get().direction())
                    % port.get().position_of_definition()
                    )
          , _memory_buffer (memory_buffer)
          , _port (port)
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

      forbidden_shadowing::forbidden_shadowing
        ( const type::structure_type& early
        , const type::structure_type& late
        , const std::string& port_name
        )
          : generic ( boost::format ( "struct %1% at %2% shadowed at %3%."
                                      " This is forbidden because it is"
                                      " the type of the port %4%."
                                    )
                    % early.name()
                    % early.position_of_definition()
                    % late.position_of_definition()
                    % port_name
                    )
      {}

      duplicate_specialize::duplicate_specialize
        ( const id::ref::specialize& early
        , const id::ref::specialize& late
        )
          : generic_duplicate<id::ref::specialize>
            (early, late, boost::format ("specialize %1%") % early.get().name())
      {}

      duplicate_place::duplicate_place ( const id::ref::place& early
                                       , const id::ref::place& late
                                       )
        : generic_duplicate<id::ref::place>
          (early, late, boost::format ("place %1%") % early.get().name())
      {}

      duplicate_transition::duplicate_transition
        ( const id::ref::transition& early
        , const id::ref::transition& late
        )
          : generic_duplicate<id::ref::transition>
            (early, late, boost::format ("transition %1%") % early.get().name())
      {}

      duplicate_port::duplicate_port ( const id::ref::port& early
                                     , const id::ref::port& late
                                     )
        : generic_duplicate<id::ref::port>
                            ( early
                            , late
                            , boost::format ("%1%-port %2%")
                            % we::type::enum_to_string (early.get().direction())
                            % early.get().name()
                            )
      {}

      duplicate_template::duplicate_template
        ( const id::ref::tmpl& early
        , const id::ref::tmpl& late
        )
          : generic_duplicate<id::ref::tmpl>
            (early, late, boost::format ("template %1%") % early.get().name())
      {}

      duplicate_place_map::duplicate_place_map
        ( const id::ref::place_map& early
        , const id::ref::place_map& late
        )
          : generic_duplicate<id::ref::place_map>
                              ( early
                              , late
                              , boost::format ("place-map %1% <-> %2%")
                              % early.get().place_virtual()
                              % early.get().place_real()
                              )
      {}

      duplicate_external_function::duplicate_external_function
         ( const id::ref::module& early
         , const id::ref::module& late
         )
          : generic_duplicate<id::ref::module>
            ( early
            , late
            , boost::format ( "external function %1% in module %2%"
                              " has conflicting definition"
                            )
            % early.get().function()
            % early.get().name()
            )
      {}

      duplicate_connect::duplicate_connect
        ( const id::ref::connect& early
        , const id::ref::connect& late
        )
          : generic_duplicate<id::ref::connect>
            ( early
            , late
            , boost::format ( "connect-%1% %2% <-> %3%"
                              " (existing connection is connect-%4%)"
                            )
            % we::edge::enum_to_string (late.get().direction())
            % late.get().place()
            % late.get().port()
            % we::edge::enum_to_string (early.get().direction())
            )
      {}

      duplicate_response::duplicate_response
        ( const id::ref::response& early
        , const id::ref::response& late
        )
          : generic_duplicate<id::ref::response>
            ( early
            , late
            , boost::format ( "connect-response %1% -> %2%"
                              " (existing response connects to %3%)"
                            )
            % late.get().port()
            % late.get().to()
            % early.get().to()
            )
      {}

      duplicate_memory_buffer::duplicate_memory_buffer
        ( id::ref::memory_buffer const& early
        , id::ref::memory_buffer const& late
        )
          : generic_duplicate<id::ref::memory_buffer>
            ( early
            , late
            , boost::format ("memory-buffer '%1%'") % late.get().name()
            )
      {}

      place_type_unknown::place_type_unknown (const id::ref::place& place)
        : generic ( boost::format ("unknown type %1% for place %2% at %3%")
                  % place.get().type()
                  % place.get().name()
                  % place.get().position_of_definition()
                  )
        , _place (place)
      {}
    }
  }
}
