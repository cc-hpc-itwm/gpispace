// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/error.hpp>

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/net.hpp>
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
        : generic ( boost::format ("%1%: missing attribute %2% in %3")
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
            % petri_net::edge::enum_to_string (late.get().direction())
            % late.get().place()
            % late.get().port()
            % petri_net::edge::enum_to_string (early.get().direction())
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
