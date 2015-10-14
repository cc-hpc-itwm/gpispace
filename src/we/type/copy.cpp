// {dimitri.blatner, mirko.rahn}@itwm.fraunhofer.de

#include <we/type/copy.hpp>

#include <we/type/id.hpp>
#include <we/type/port.hpp>
#include <we/type/requirement.hpp>

#include <fhg/util/boost/variant.hpp>

#include <iostream>
#include <list>
#include <set>

namespace we
{
  namespace type
  {
    namespace
    {
      struct equal_to
      {
        equal_to (place_id_type const& pid)
        : _pid (pid)
        {}

        template<typename T>
          bool operator() (T x) const
        {
          return _pid == x.get_left();
        }

      private:
        place_id_type const _pid;
      };

      // identifying transformed
      const std::string transformation_property ("_TRANSFORMATION");
      const std::string transformation_property_value ("true");
    }

    std::tuple< net_type
              , std::unordered_map<place_id_type, place_id_type>
              , place_id_type
              >
      copy ( net_type const& input_net
           , boost::optional<std::string> control
           , boost::optional<std::vector<std::string>> break_after_transition
           , we::type::property::type transformation_properties
           )
    {
      net_type new_net;

      std::unordered_map<transition_id_type, transition_id_type>
        transition_id;
      std::unordered_map<place_id_type, place_id_type> place_id;

      for ( std::pair<place_id_type, place::type> const& in_place
          : input_net.places()
          )
      {
        place_id_type const pid_input_net (in_place.first);
        place_id_type const pid_new_net (new_net.add_place (in_place.second));

        place_id.emplace (pid_input_net, pid_new_net);

        for ( pnet::type::value::value_type const& value
            : input_net.get_token (pid_input_net)
            )
        {
          new_net.put_value (pid_new_net, value);
        }
      }

      // create new global control place if desired
      place_id_type control_place_id;

      if (control)
      {
        control_place_id =
          new_net.add_place (place::type ( *control
                                         , std::string ("control")
                                         , boost::none
                                         , transformation_properties
                                         )
                            );
      }

      for ( std::pair<transition_id_type, transition_t> const& input_tid_trans
          : input_net.transitions()
          )
      {
        transition_id_type const input_net_tid (input_tid_trans.first);

        transition_t input_net_transition (input_tid_trans.second);

        // create input port for connecting the global control place
        port_id_type new_port_id;

        if (control)
        {
          new_port_id = input_net_transition
            .add_port (port_t ( *control
                              , we::type::PORT_IN
                              , std::string ("control")
                              , transformation_properties
                              )
                      );
        }

        transition_id_type const new_tid
          = new_net.add_transition (input_net_transition);

        // connect transition with control place
        if (control)
        {
          if (!!break_after_transition)
          {
            new_net.add_connection
              ( (std::find ( break_after_transition->begin()
                           , break_after_transition->end()
                           , input_net_transition.name()
                           )
                           != break_after_transition->end()
                )
                ? we::edge::PT
                : we::edge::PT_READ
              , new_tid
              , control_place_id
              , new_port_id
              , transformation_properties
              );
          }
          else // step function
          {
            new_net.add_connection ( we::edge::PT
                                   , new_tid
                                   , control_place_id
                                   , new_port_id
                                   , transformation_properties
                                   );
          }
        }

        transition_id.emplace (input_net_tid, new_tid);
      }

      // outgoing connections, transition ports to places
      for ( std::pair< transition_id_type
                     , net_type::port_to_place_with_info_type
                     > const& tppi
          : input_net.port_to_place()
          )
      {
        for ( net_type::port_to_place_with_info_type::value_type const& ppi
            : tppi.second
            )
        {
          new_net.add_connection ( edge::TP
                                 , transition_id.at (tppi.first)
                                 , place_id.at (ppi.get_right())
                                 , ppi.get_left() // new port id equals old one
                                 , ppi.info
                                 );
        }
      }

      // incoming connections, places to transition ports
      for ( std::pair< transition_id_type
                     , net_type::place_to_port_with_info_type
                     > const& tppi
          : input_net.place_to_port()
          )
      {
        transition_id_type const tid (tppi.first);

        auto const place_ids_consume
          (input_net.place_to_transition_consume().right.equal_range (tid));

        for ( net_type::place_to_port_with_info_type::value_type const& ppi
            : tppi.second
            )
        {
          place_id_type const input_net_place_id (ppi.get_left());

          new_net.add_connection
            ( std::find_if ( place_ids_consume.first  // iterator begin
                           , place_ids_consume.second // iterator end
                           , equal_to (input_net_place_id)
                           )
                           != place_ids_consume.second
              ? edge::PT : edge::PT_READ
            , transition_id.at (tid)
            , place_id.at (input_net_place_id)
            , ppi.get_right()
            , ppi.info
            );
        }
      }

      return std::make_tuple(new_net, place_id, control_place_id);
    }

    activity_t copy
      ( activity_t const& input_activity
      , boost::optional<std::string> control
      , boost::optional<std::vector<std::string>> break_after_transition
      )
    {
      if (!input_activity.transition().net())
      {
        return input_activity;
      }

      // property identifying transformations
      we::type::property::type transformation_properties;
      std::list<std::string> const property_path {transformation_property};
      transformation_properties.set ( property_path
                                    , transformation_property_value
                                    );

      std::tuple< net_type
                , std::unordered_map<place_id_type, place_id_type>
                , place_id_type
                >
        const copy_map (copy ( input_activity.transition().net().get()
                             , control
                             , break_after_transition
                             , transformation_properties
                             )
                       );

      transition_t new_trans ( input_activity.transition().name()
                             , std::get<0>(copy_map)
                             , input_activity.transition().condition()
                             , input_activity.transition().is_internal()
                             , input_activity.transition().prop()
                             , input_activity.transition().priority()
                             );

      for ( requirement_t const& req
          : input_activity.transition().requirements())
      {
        new_trans.add_requirement (req);
      }

      // handle ports, use std::get<1>(copy_map) to adjust associations
      std::unordered_map<port_id_type, port_id_type> port_id;

      for ( std::pair<port_id_type, port_t> const input_port
          : input_activity.transition().ports_input()
          )
      {
        port_t const port (input_port.second);

        port_id.emplace
          ( input_port.first
          , new_trans.add_port
            ( port.associated_place()
            ? port_t ( port.name()
                     , port.direction()
                     , port.signature()
                     , std::get<1>(copy_map).at (port.associated_place().get())
                     , port.property()
                     )
            : port
            )
          );
      }

      for ( std::pair<port_id_type, port_t> const output_port
          : input_activity.transition().ports_output()
          )
      {
        port_t const port (output_port.second);

        port_id.emplace
          ( output_port.first
          , new_trans.add_port
            ( port.associated_place()
            ? port_t ( port.name()
                     , port.direction()
                     , port.signature()
                     , std::get<1>(copy_map).at (port.associated_place().get())
                     , port.property()
                     )
            : port
            )
          );
      }

      for ( std::pair<port_id_type, port_t> const tunnel_port
          : input_activity.transition().ports_tunnel()
          )
      {
        port_t const port (tunnel_port.second);

        port_id.emplace
          ( tunnel_port.first
          , new_trans.add_port
            ( port.associated_place()
            ? port_t ( port.name()
                     , port.direction()
                     , port.signature()
                     , std::get<1>(copy_map).at (port.associated_place().get())
                     , port.property()
                     )
            : port
            )
          );
      }

      // new input port for control place
      if (control)
      {
        new_trans.add_port (port_t (*control
                                   , we::type::PORT_IN
                                   , std::string ("control")
                                   , std::get<2>(copy_map) // control_place_id
                                   , transformation_properties
                                   )
                           );
      }

      activity_t new_activity (new_trans, input_activity.transition_id());

      for ( std::pair<pnet::type::value::value_type, port_id_type> const& top
          : input_activity.input()
          )
      {
        new_activity.add_input (port_id.at (top.second), top.first);
      }

      for ( std::pair<pnet::type::value::value_type, port_id_type> const& top
          : input_activity.output()
          )
      {
        new_activity.add_output (port_id.at (top.second), top.first);
      }

      return new_activity;
    }

    std::tuple< transition_t
              , std::set<port_id_type>
              , std::unordered_map<port_id_type, port_id_type>
              >
      copy_rev (transition_t const& input_transition)
    {
      std::unordered_map<port_id_type, port_id_type> port_id;

      transition_t new_transition ( input_transition.name()
                                  , input_transition.data()
                                  , input_transition.condition()
                                  , input_transition.is_internal()
                                  , input_transition.prop()
                                  , input_transition.priority()
                                  );

      for (requirement_t const& req : input_transition.requirements())
      {
        new_transition.add_requirement (req);
      }

      std::list<std::string> const transformation_property_path
        {transformation_property};

      // save skipped port ids for handling connetctions later
      std::set<port_id_type> removed_port_ids;

      // copy ports
      for ( std::pair<port_id_type, port_t> const input_port
          : input_transition.ports_input()
          )
      {
        port_t const in_port (input_port.second);

        // skip transformation input ports
        boost::optional<const pnet::type::value::value_type&> in_transf_prop
          = in_port.property().get (transformation_property_path);

        if (!in_transf_prop)
        {
          port_id.emplace (input_port.first, new_transition.add_port (in_port));
        }
        else
        {
          removed_port_ids.insert (input_port.first);
        }
      }

      for ( std::pair<port_id_type, port_t> const output_port
          : input_transition.ports_output()
          )
      {
        port_t const out_port (output_port.second);

        port_id.emplace (output_port.first, new_transition.add_port (out_port));
      }

      for ( std::pair<port_id_type, port_t> const tunnel_port
          : input_transition.ports_tunnel()
          )
      {
        port_t const tun_port (tunnel_port.second);

        port_id.emplace (tunnel_port.first, new_transition.add_port (tun_port));
      }

      return std::make_tuple(new_transition, removed_port_ids, port_id);
    }

    std::tuple< net_type
              , std::unordered_map<place_id_type, place_id_type>
              , place_id_type
              >
      copy_rev ( net_type const& input_net
               , boost::optional<std::string> control
               )
    {
      net_type new_net;

      std::unordered_map<transition_id_type, transition_id_type>
        transition_id;
      std::unordered_map<place_id_type, place_id_type> place_id;

      std::list<std::string> const transformation_property_path
        {transformation_property};

      place_id_type control_place_id;

      // copy all places and tokens but transformed places
      for ( std::pair<place_id_type, place::type> const& input_place
          : input_net.places()
          )
      {
        place_id_type const pid_input_net (input_place.first);
        place_id_type pid_new_net;

        boost::optional<const pnet::type::value::value_type&>
          transformed
            = input_place.second.property().get (transformation_property_path);

        // skip if transformed
        if ( !control
           || *control != input_place.second.name()
           || !transformed
           || fhg::util::boost::get_or_none<std::string>
                (transformed.get()).get()
              != transformation_property_value
           )
        {
          pid_new_net = new_net.add_place (input_place.second);

          place_id.emplace (pid_input_net, pid_new_net);

          for ( pnet::type::value::value_type const& value
              : input_net.get_token (pid_input_net)
              )
          {
            new_net.put_value (pid_new_net, value);
          }
        }
        else // save control place id for handling connections later
        {
          control_place_id = pid_input_net;
        }
      }

      std::unordered_map<transition_id_type, std::set<port_id_type>>
        transformation_ports;

      std::unordered_map< transition_id_type
                        , std::unordered_map<port_id_type, port_id_type>
                        > transition_port_id_map;

      // copy all transitions, save their port mapping and transformation ports
      for ( std::pair<transition_id_type, transition_t> const& in_tid_trans
          : input_net.transitions()
          )
      {
        transition_id_type const input_net_tid (in_tid_trans.first);
        transition_t input_net_transition (in_tid_trans.second);

        std::tuple< transition_t
                  , std::set<port_id_type>
                  , std::unordered_map<port_id_type, port_id_type>
                  >
          const new_transition_information = copy_rev (input_net_transition);

        transition_port_id_map.emplace
          (input_net_tid, std::get<2> (new_transition_information));

        transition_id_type const new_tid =
          new_net.add_transition (std::get<0> (new_transition_information));

        transformation_ports.emplace
          (input_net_tid, std::get<1>(new_transition_information));

        transition_id.emplace (input_net_tid, new_tid);
      }

      // connect transition ports to places (outgoing)
      for ( std::pair< transition_id_type
                     , net_type::port_to_place_with_info_type
                     > const& tppi
          : input_net.port_to_place()
          )
      {
        for ( net_type::port_to_place_with_info_type::value_type const& ppi
            : tppi.second
            )
        {
          new_net.add_connection
            ( edge::TP
            , transition_id.at (tppi.first)
            , place_id.at (ppi.get_right())
            , transition_port_id_map.at (tppi.first).at (ppi.get_left())
            , ppi.info
            );
        }
      }

      // connect transition places to ports (incoming), skip transformed place
      for ( std::pair< transition_id_type
                     , net_type::place_to_port_with_info_type
                     > const& tppi
          : input_net.place_to_port()
          )
      {
        transition_id_type const tid (tppi.first);

        auto const place_ids_consume
          (input_net.place_to_transition_consume().right.equal_range (tid));

        for ( net_type::place_to_port_with_info_type::value_type const& ppi
            : tppi.second
            )
        {
          place_id_type const input_net_place_id (ppi.get_left());

          if (input_net_place_id != control_place_id)
          {
            new_net.add_connection
              ( std::find_if ( place_ids_consume.first  // iterator begin
                             , place_ids_consume.second // iterator end
                             , equal_to (input_net_place_id)
                             )
                             != place_ids_consume.second
                ? edge::PT : edge::PT_READ
              , transition_id.at (tid)
              , place_id.at (input_net_place_id)
              , transition_port_id_map.at (tid).at (ppi.get_right())
              , ppi.info
              );
          }
        }
      }

      return std::make_tuple(new_net, place_id, control_place_id);
    }

    activity_t copy_rev ( activity_t const& input_activity
                        , boost::optional<std::string> control
                        )
    {
      if (!input_activity.transition().net())
      {
        return input_activity;
      }

      std::tuple< net_type
                , std::unordered_map<place_id_type, place_id_type>
                , place_id_type
                >
        const copy_map (copy_rev ( input_activity.transition().net().get()
                                 , control
                                 )
                       );

      transition_t new_transition ( input_activity.transition().name()
                                  , std::get<0>(copy_map)
                                  , input_activity.transition().condition()
                                  , input_activity.transition().is_internal()
                                  , input_activity.transition().prop()
                                  , input_activity.transition().priority()
                                  );

      for ( requirement_t const& req
          : input_activity.transition().requirements()
          )
      {
        new_transition.add_requirement (req);
      }

      // handle ports, use copy_map.second to adjust associations
      std::unordered_map<port_id_type, port_id_type> port_id;

      port_id_type control_port_id;

      for ( std::pair<port_id_type, port_t> const input_port
          : input_activity.transition().ports_input()
          )
      {
        port_t const port (input_port.second);

        // don't copy input port for global control places
        std::list<std::string> const transformation_property_path
          {transformation_property};

        boost::optional<const pnet::type::value::value_type&>
          transformed =
            port.property().get (transformation_property_path);

        if (  !transformed
           || !( port.associated_place()
               && port.associated_place().get() == std::get<2>(copy_map)
               )
           )
        {
          port_id.emplace
            ( input_port.first
            , new_transition.add_port
                ( port.associated_place()
                ? port_t ( port.name()
                         , port.direction()
                         , port.signature()
                         , std::get<1>(copy_map).at
                             (port.associated_place().get())
                         , port.property()
                         )
                : port
                )
            );
        }
        else // save control place id for skipping its tokens
        {
          control_port_id = input_port.first;
        }
      }

      for ( std::pair<port_id_type, port_t> const output_port
          : input_activity.transition().ports_output()
          )
      {
        port_t const port (output_port.second);

        port_id.emplace
          ( output_port.first
          , new_transition.add_port
              ( port.associated_place()
              ? port_t ( port.name()
                       , port.direction()
                       , port.signature()
                       , std::get<1>(copy_map).at
                           (port.associated_place().get())
                       , port.property()
                       )
              : port
              )
          );
      }

      for ( std::pair<port_id_type, port_t> const tunnel_port
          : input_activity.transition().ports_tunnel()
          )
      {
        port_t const port (tunnel_port.second);

        port_id.emplace
          ( tunnel_port.first
          , new_transition.add_port
              ( port.associated_place()
              ? port_t ( port.name()
                       , port.direction()
                       , port.signature()
                       , std::get<1>(copy_map).at
                           (port.associated_place().get())
                       , port.property()
                       )
              : port
              )
          );
      }

      activity_t new_act (new_transition, input_activity.transition_id());

      // copy tokens for ports
      for ( std::pair<pnet::type::value::value_type, port_id_type> const& input
          : input_activity.input()
          )
      {
        if (input.second != control_port_id)
        {
          new_act.add_input (port_id.at (input.second), input.first);
        }
      }

      for ( std::pair<pnet::type::value::value_type, port_id_type> const& output
          : input_activity.output()
          )
      {
        new_act.add_output (port_id.at (output.second), output.first);
      }

      return new_act;
    }
  }
}
