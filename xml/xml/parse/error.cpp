// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/error.hpp>

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/place.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/port.hpp>
#include <xml/parse/type/template.hpp>
#include <xml/parse/type/transition.hpp>

#include <we/type/net.hpp>

#include <boost/format.hpp>

namespace xml
{
  namespace parse
  {
    namespace error
    {
      duplicate_port::duplicate_port ( const id::ref::port& port
                                     , const id::ref::port& old_port
                                     , const boost::filesystem::path & path
                                     )
        : generic ( boost::format ("duplicate %1%-port %2% in %3%")
                  % we::type::enum_to_string (port.get().direction())
                  % port.get().name()
                  % path
                  )
        , _port (port)
        , _old_port (old_port)
        , _path (path)
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

      duplicate_connect::duplicate_connect
        ( const id::ref::connect& connection
        , const id::ref::connect& old_connection
        , const id::ref::transition& transition
        )
          : generic ( boost::format ( "duplicate connect-%1% %2% <-> %3% "
                                      "for transition %4% in %5%"
                                      " (existing connection is connect-%6%)"
                                      " in %7% and %8%"
                                    )
                    % petri_net::edge::enum_to_string
                      (connection.get().direction())
                    % connection.get().place()
                    % connection.get().port()
                    % transition.get().name()
                    % transition.get().position_of_definition()
                    % petri_net::edge::enum_to_string
                      (old_connection.get().direction())
                    % connection.get().position_of_definition()
                    % old_connection.get().position_of_definition()
                    )
          , _connection (connection)
          , _old_connection (old_connection)
          , _transition (transition)
      { }

      duplicate_place_map::duplicate_place_map
        ( const id::ref::place_map& place_map
        , const id::ref::place_map& old_place_map
        , const id::ref::transition& transition
        )
          : generic ( boost::format ( "duplicate place-map %1% <-> %2% for "
                                      "transition %3% in %4%"
                                    )
                    % place_map.get().place_virtual()
                    % place_map.get().place_real()
                    % transition.get().name()
                    % transition.get().position_of_definition()
                    )
          , _place_map (place_map)
          , _old_place_map (old_place_map)
          , _transition (transition)
      { }

      duplicate_transition::duplicate_transition
        ( const id::ref::transition& transition
        , const id::ref::transition& old_transition
        )
          : generic ( boost::format ( "duplicate transition %1% in %2%. "
                                      "first definition was in %3%"
                                    )
                    % transition.get().name()
                    % transition.get().position_of_definition()
                    % old_transition.get().position_of_definition()
                    )
          , _transition (transition)
          , _old_transition (old_transition)
      { }

      duplicate_function::duplicate_function
        ( const id::ref::function& function
        , const id::ref::function& old_function
        )
          : generic ( boost::format ( "duplicate function %1% in %2%. "
                                      "first definition was in %3%"
                                    )
                    % function.get().name()
                    % function.get().path
                    % old_function.get().path
                    )
          , _function (function)
          , _old_function (old_function)
      { }

      duplicate_template::duplicate_template
        ( const id::ref::tmpl& tmpl
        , const id::ref::tmpl& old_template
        )
          : generic ( boost::format ( "duplicate template %1% in %2%. "
                                      "first definition was in %3%"
                                    )
                    % tmpl.get().name()
                    % tmpl.get().path()
                    % old_template.get().path()
                    )
          , _template (tmpl)
          , _old_template (old_template)
      { }

      parse_link_prefix::parse_link_prefix ( const std::string& reason
                                           , const std::string& input
                                           , const std::size_t& pos
                                           )
        : generic ( boost::format ( "could not parse link_prefix [%1%]:"
                                    " %2%, input was %3%"
                                  )
                  % pos
                  % reason
                  % input
                  )
        , _reason (reason)
        , _input (input)
          //        , _pos (pos)
      {}

      link_prefix_missing::link_prefix_missing (const std::string& key)
        : generic ( boost::format ("missing binding for key %1% in link prefix")
                  % key
                  )
        , _key (key)
      {}

      duplicate_external_function::duplicate_external_function
        (const id::ref::module& mod, const id::ref::module& old)
          : generic ( boost::format ( "the external function %1% in module %2%"
                                      " has different definitions in %3% and"
                                      " %4%"
                                    )
                    % mod.get().function
                    % mod.get().name()
                    % old.get().position_of_definition()
                    % mod.get().position_of_definition()
                    )
          , _mod (mod)
          , _old (old)
      {}

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
                   % place.get().signature()
                   % place.get().position_of_definition()
                   % port.get().name()
                   % port.get().type()
                   % port.get().signature()
                   % port.get().position_of_definition()
                   % transition.get().name()
                   % connection.get().position_of_definition()
                   )
          , _transition (transition)
          , _connection (connection)
          , _port (port)
          , _place (place)
        {}
    }
  }
}
