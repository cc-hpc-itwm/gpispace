// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/weaver/display.hpp>
#include <pnete/weaver/weaver.hpp>

#include <pnete/ui/graph/scene.hpp>
#include <pnete/ui/graph/transition.hpp>
#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/place.hpp>

#include <xml/parse/types.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace weaver
    {
      function::function ( function_with_mapping_type function_with_mapping
                         , data::internal_type* root
                         )
        : _proxy (NULL)
        , _function_with_mapping (function_with_mapping)
        , _scene (NULL)
        , _root (root)
      {
        FROM (function<function> (this, _function_with_mapping.function()));
      }
      data::proxy::type* function::proxy () const { return _proxy; }
      XMLTYPE(ports_type)& function::in () { return _ports.in; }
      XMLTYPE(ports_type)& function::out () { return _ports.out; }

      WSIG(function, expression::open, XMLTYPE(expression_type), exp)
      {
        _proxy = new data::proxy::type
          ( data::proxy::expression_proxy
            ( _root
            , data::proxy::data::expression_type
              ( const_cast< XMLTYPE(expression_type) &> (exp)
              , in()
              , out()
              )
            , _function_with_mapping
            )
          );
      }
      WSIG(function, mod::open, XMLTYPE(mod_type), mod)
      {
        _proxy = new data::proxy::type
          ( data::proxy::mod_proxy
            ( _root
            , data::proxy::data::mod_type
              ( const_cast< XMLTYPE(mod_type) &> (mod)
              , in()
              , out()
              )
            , _function_with_mapping
            )
          );
      }
      WSIG(function, net::open, XMLTYPE(net_type), net)
      {
        _scene = new ui::graph::scene(const_cast< XMLTYPE(net_type) &> (net));
        _proxy = new data::proxy::type
          ( data::proxy::net_proxy
            ( _root
            , data::proxy::data::net_type
              ( const_cast< XMLTYPE(net_type) &> (net)
              )
            , _function_with_mapping
            , _scene
            )
          );

        weaver::net wn ( _root
                       , _scene
                       , const_cast< XMLTYPE(net_type) &> (net)
                       , in()
                       , out()
                       );

        FROM(net) (&wn, net);
      }
      WSIG(function, function::in, XMLTYPE(ports_type), in)
      {
        _ports.in = in;
      }
      WSIG(function, function::out, XMLTYPE(ports_type), out)
      {
        _ports.out = out;
      }
      WSIG(function, function::fun, XMLTYPE(function_type::type), fun)
      {
        boost::apply_visitor (FROM(visitor::net_type<function>) (this), fun);
      }


      transition::transition ( data::internal_type* root
                             , ui::graph::scene* scene
                             , ui::graph::transition* transition
                             , XMLTYPE(net_type)& net
                             , item_by_name_type& place_item_by_name
                             )
        : _scene (scene)
        , _transition (transition)
        , _current_port_direction ()
        , _net (net)
        , _place_item_by_name (place_item_by_name)
        , _port_in_item_by_name ()
        , _port_out_item_by_name ()
        , _root (root)
        , _type_map (boost::none)
      {}
      function_with_mapping_type
      transition::get_function (XMLTYPE(transition_type::f_type)& f)
      {
        return boost::apply_visitor (visitor::get_function(_net), f);
      }

      WSIG(transition, transition::name, std::string, name)
      {
        _transition->name (QString::fromStdString (name));
      }
      WSIG( transition
          , transition::function
          , XMLTYPE(transition_type::f_type)
          , fun
          )
      {
        function_with_mapping_type function_with_mapping
          (get_function (const_cast<XMLTYPE(transition_type::f_type)&>(fun)));

        weaver::function sub (function_with_mapping, _root);

        _type_map = function_with_mapping.type_map();

        _transition->proxy (sub.proxy());

        _current_port_direction = ui::graph::port::IN;
        from::many (this, sub.in(), FROM(port));

        _current_port_direction = ui::graph::port::OUT;
        from::many (this, sub.out(), FROM(port));
      }
      WSIG(transition, port::open, ITVAL(XMLTYPE(ports_type)), port)
      {
        ui::graph::port* port_item
          (new ui::graph::port ( _current_port_direction
                               , _type_map
                               , _transition
                               )
          );

        weaver::port wp ( port_item
                        , _current_port_direction == ui::graph::port::IN
                        ? _port_in_item_by_name
                        : _port_out_item_by_name
                        );

        FROM(port) (&wp, port);
      }
      WSIGE(transition, transition::close)
      {
        _transition->repositionChildrenAndResize();
      }
      WSIG(transition, transition::connect_read, XMLTYPE(connections_type), cs)
      {
        weaver::connection wc ( _scene
                              , _place_item_by_name
                              , _port_in_item_by_name
                              , ui::graph::port::IN
                              , true
                              );

        from::many (&wc, cs, FROM(connection));
      }
      WSIG(transition, transition::connect_in, XMLTYPE(connections_type), cs)
      {
        weaver::connection wc ( _scene
                              , _place_item_by_name
                              , _port_in_item_by_name
                              , ui::graph::port::IN
                              );

        from::many (&wc, cs, FROM(connection));
      }
      WSIG(transition, transition::connect_out, XMLTYPE(connections_type), cs)
      {
        weaver::connection wc ( _scene
                              , _place_item_by_name
                              , _port_out_item_by_name
                              , ui::graph::port::OUT
                              );

        from::many (&wc, cs, FROM(connection));
      }
      WSIG(transition, transition::properties, WETYPE(property::type), props)
      {
        weaver::property wp (_transition);

        FROM (properties) (&wp, props);
      }

      property::property (ui::graph::item* item)
        : _item (item)
        , _path ()
      {}
      WSIG(property, properties::open, WETYPE(property::type), props)
      {
        from::many (this, props.get_map(), FROM(property));
      }
      WSIG(property, property::open, WETYPE(property::key_type), key)
      {
        _path.push_back (key);
      }
      WSIGE(property, property::close)
      {
        _path.pop_back();
      }
      WSIG(property, property::value, WETYPE(property::value_type), value)
      {
        if (_path.size() > 1 && _path[0] == "fhg" && _path[1] == "pnete")
          {
            if (_path.size() > 2 && _path[2] == "position")
              {
                if (_path.size() > 3)
                  {
                    if (_path[3] == "x")
                      {
                        _item->setPos
                          ( boost::lexical_cast<float>(value)
                          , _item->pos().y()
                          );
                      }
                    else if (_path[3] == "y")
                      {
                        _item->setPos
                          ( _item->pos().x()
                          , boost::lexical_cast<float>(value)
                          );
                      }
                  }
              }
          }
      }

      connection::connection ( ui::graph::scene* scene
                             , item_by_name_type& place_item_by_name
                             , item_by_name_type& port_item_by_name
                             , const ui::graph::port::DIRECTION& direction
                             , const bool& read
                             )
        : _scene (scene)
        , _place_item_by_name (place_item_by_name)
        , _port_item_by_name (port_item_by_name)
        , _direction (direction)
        , _read (read)
        , _port ()
        , _place ()
      {}

      WSIG(connection, connection::port, std::string, port)
      {
        _port = port;
      }
      WSIG(connection, connection::place, std::string, place)
      {
        _place = place;
      }
      WSIGE(connection, connection::close)
      {
        typedef item_by_name_type::iterator iterator_type;

        const iterator_type port_pos (_port_item_by_name.find (_port));
        const iterator_type place_pos (_place_item_by_name.find (_place));

        if (port_pos == _port_item_by_name.end())
          {
            throw
              std::runtime_error ("connection: port " + _port + " not found");
          }
        if (place_pos == _place_item_by_name.end())
          {
            throw
              std::runtime_error ("connection: place " + _place + " not found");
          }

        if (_direction == ui::graph::port::IN)
          {
            _scene->create_connection ( place_pos->second
                                      , port_pos->second
                                      , _read
                                      );
          }
        else
          {
            _scene->create_connection ( port_pos->second
                                      , place_pos->second
                                      , _read
                                      );
          }
      }

      net::net ( data::internal_type* root
               , ui::graph::scene* scene
               , XMLTYPE(net_type)& net
               , XMLTYPE(ports_type)& in
               , XMLTYPE(ports_type)& out
               )
        : _scene (scene)
        , _net (net)
        , _in (in)
        , _out (out)
        , _place_item_by_name ()
        , _root ()
      {}
      WSIGE(net, net::close)
      {
        {
          weaver::port_toplevel wptl ( _scene
                                     , ui::graph::port::OUT
                                     , _place_item_by_name
                                     );
          from::many (&wptl, _in, FROM(port));
        }

        {
          weaver::port_toplevel wptl ( _scene
                                     , ui::graph::port::IN
                                     , _place_item_by_name
                                     );
          from::many (&wptl, _out, FROM(port));
        }
      }
      WSIG(net, net::transitions, XMLTYPE(net_type::transitions_type), transitions)
      {
        from::many (this, transitions, FROM(transition));
      }
      WSIG(net, net::places, XMLTYPE(net_type::places_type), places)
      {
        from::many (this, places, FROM(place));
      }
      WSIG(net, place::open, ITVAL(XMLTYPE(net_type::places_type)), place)
      {
        ui::graph::place* place_item (new ui::graph::place());
        weaver::place wp (place_item, _place_item_by_name);
        _scene->addItem (place_item);
        FROM(place) (&wp, place);
      }
      WSIG( net
          , transition::open
          , ITVAL(XMLTYPE(net_type::transitions_type))
          , transition
          )
      {
        ui::graph::transition* trans
          ( new ui::graph::transition
            (const_cast<ui::graph::transition::transition_type &> (transition))
          );
        _scene->addItem (trans);
        weaver::transition wt ( _root
                              , _scene
                              , trans
                              , _net
                              , _place_item_by_name
                              );
        FROM(transition) (&wt, transition);
      }


      port::port ( ui::graph::port* port
                 , item_by_name_type& port_item_by_name
                 )
        : _port (port)
        , _port_item_by_name (port_item_by_name)
      {}

      WSIG(port, port::name, std::string, name)
      {
        _port->name (QString::fromStdString (name));
        _port_item_by_name[name] = _port;
      }
      WSIG(port, port::type, std::string, type)
      {
        _port->we_type (QString::fromStdString (type));
      }


      place::place ( ui::graph::place* place
                   , item_by_name_type& place_item_by_name
                   )
        : _place (place)
        , _place_item_by_name (place_item_by_name)
      {}

      WSIG(place, place::name, std::string, name)
      {
        _place->name (QString::fromStdString (name));
        _place_item_by_name[name] = _place;
      }
      WSIG(place, place::type, std::string, type)
      {
        _place->we_type (QString::fromStdString (type));
      }
      WSIG(place, place::properties, WETYPE(property::type), props)
      {
        weaver::property wp (_place);

        FROM(properties) (&wp, props);
      }

      port_toplevel::port_toplevel
        ( ui::graph::scene* scene
        , const ui::graph::port::DIRECTION& direction
        , item_by_name_type& place_item_by_name
        )
          : _scene (scene)
          , _place_item_by_name (place_item_by_name)
          , _name ()
          , _direction (direction)
          , _port_item ()
      {}

      WSIG(port_toplevel, port::open, ITVAL(XMLTYPE(ports_type)), port)
      {
        _port_item = new ui::graph::port (_direction);
        _scene->addItem (_port_item);
      }
      WSIG(port_toplevel, port::name, std::string, name)
      {
        _name = name;
        _port_item->name (QString::fromStdString (name));
      }
      WSIG(port_toplevel, port::type, std::string, type)
      {
        _port_item->we_type (QString::fromStdString (type));
      }
      WSIG(port_toplevel, port::place, MAYBE(std::string), place)
      {
        if (place)
          {
            item_by_name_type _port_item_by_name;

            _port_item_by_name[_name] = _port_item;

            weaver::connection wc ( _scene
                                  , _place_item_by_name
                                  , _port_item_by_name
                                  , _direction
                                  );

            FROM(connection) (&wc, XMLTYPE(connect_type) (*place, _name));
          }
      }
    }
  }
}

