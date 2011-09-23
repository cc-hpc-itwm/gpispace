// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/weaver/display.hpp>
#include <pnete/weaver/weaver.hpp>

#include <pnete/ui/graph/scene.hpp>
#include <pnete/ui/graph/transition.hpp>
#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/place.hpp>

#include <xml/parse/types.hpp>

#include <iostream>

namespace fhg
{
  namespace pnete
  {
    namespace weaver
    {
      function::function (XMLTYPE(function_type) & fun)
        : _proxy (NULL)
        , _fun (fun)
        , _scene (NULL)
      {
        FROM (function<function> (this, fun));
      }
      data::proxy::type* function::proxy () const { return _proxy; }
      XMLTYPE(ports_type)& function::in () { return _ports.in; }
      XMLTYPE(ports_type)& function::out () { return _ports.out; }

      WSIG(function, expression::open, XMLTYPE(expression_type), exp)
      {
        _proxy = new data::proxy::type
          ( data::proxy::expression_proxy
            ( data::proxy::data::expression_type
              ( const_cast< XMLTYPE(expression_type) &> (exp)
              , in()
              , out()
              )
            , _fun
            )
          );
      }
      WSIG(function, mod::open, XMLTYPE(mod_type), mod)
      {
        _proxy = new data::proxy::type
          ( data::proxy::mod_proxy
            ( data::proxy::data::mod_type
              ( const_cast< XMLTYPE(mod_type) &> (mod)
              , in()
              , out()
              )
            , _fun
            )
          );
      }
      WSIG(function, net::open, XMLTYPE(net_type), net)
      {
        _scene = new ui::graph::scene(const_cast< XMLTYPE(net_type) &> (net));
        _proxy = new data::proxy::type
          ( data::proxy::net_proxy
            ( data::proxy::data::net_type
              ( const_cast< XMLTYPE(net_type) &> (net)
              )
            , _fun
            , _scene
            )
          );

        weaver::net wn ( _scene
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


      transition::transition ( ui::graph::scene* scene
                             , ui::graph::transition* transition
                             , XMLTYPE(net_type)& net
                             , item_by_name_type& place_item_by_name
                             )
        : _scene (scene)
        , _transition (transition)
        , _current_port_direction ()
        , _net (net)
        , _place_item_by_name (place_item_by_name)
        , _port_item_by_name ()
      {}
      XMLTYPE(function_type)&
      transition::get_function (const XMLTYPE(transition_type::f_type) & f)
      {
        return boost::apply_visitor (visitor::get_function(_net), f);
      }

      WSIG(transition, transition::name, std::string, name)
      {
        _transition->name (QString(name.c_str()));
      }
      WSIG( transition
          , transition::function
          , XMLTYPE(transition_type::f_type)
          , fun
          )
      {
        function sub (get_function (fun));

        _transition->proxy (sub.proxy());

        _current_port_direction = ui::graph::port::IN;
        from::many (this, sub.in(), FROM(port));

        _current_port_direction = ui::graph::port::OUT;
        from::many (this, sub.out(), FROM(port));
      }
      WSIG(transition, port::open, ITVAL(XMLTYPE(ports_type)), port)
      {
        ui::graph::port* port_item
          (new ui::graph::port (_current_port_direction, _transition));

        weaver::port wp (port_item, _port_item_by_name);

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
                              , _port_item_by_name
                              , ui::graph::port::IN
                              , true
                              );

        from::many (&wc, cs, FROM(connection));
      }
      WSIG(transition, transition::connect_in, XMLTYPE(connections_type), cs)
      {
        weaver::connection wc ( _scene
                              , _place_item_by_name
                              , _port_item_by_name
                              , ui::graph::port::IN
                              );

        from::many (&wc, cs, FROM(connection));
      }
      WSIG(transition, transition::connect_out, XMLTYPE(connections_type), cs)
      {
        weaver::connection wc ( _scene
                              , _place_item_by_name
                              , _port_item_by_name
                              , ui::graph::port::OUT
                              );

        from::many (&wc, cs, FROM(connection));
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
        , _port ()
        , _direction (direction)
        , _read (read)
      {}

      WSIG(connection, connection::port, std::string, port)
      {
        _port = port;
      }
      WSIG(connection, connection::place, std::string, place)
      {
        if (_direction == ui::graph::port::IN)
          {
            _scene->create_connection ( _place_item_by_name[place]
                                      , _port_item_by_name[_port]
                                      , _read
                                      );
          }
        else
          {
            _scene->create_connection ( _port_item_by_name[_port]
                                      , _place_item_by_name[place]
                                      , _read
                                      );
          }
      }

      net::net ( ui::graph::scene* scene
               , XMLTYPE(net_type)& net
               , XMLTYPE(ports_type)& in
               , XMLTYPE(ports_type)& out
               )
        : _scene (scene)
        , _net (net)
        , _place_item_by_name ()
      {
        {
          weaver::port_toplevel wptl (_scene, ui::graph::port::OUT);
          from::many (&wptl, in, FROM(port));
        }

        {
          weaver::port_toplevel wptl (_scene, ui::graph::port::IN);
          from::many (&wptl, out, FROM(port));
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
        weaver::transition wt (_scene, trans, _net, _place_item_by_name);
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
        _port->name (QString (name.c_str()));
        _port_item_by_name[name] = _port;
      }
      WSIG(port, port::type, std::string, type)
      {
        _port->we_type (QString (type.c_str()));
      }


      place::place ( ui::graph::place* place
                   , item_by_name_type& place_item_by_name
                   )
        : _place (place)
        , _place_item_by_name (place_item_by_name)
      {}

      WSIG(place, place::name, std::string, name)
      {
        _place->name (QString (name.c_str()));
        _place_item_by_name[name] = _place;
      }
      WSIG(place, place::type, std::string, type)
      {
        _place->we_type (QString (type.c_str()));
      }


      port_toplevel::port_toplevel
        ( ui::graph::scene* const scene
        , const ui::graph::port::DIRECTION& current_direction
        )
          : _scene (scene)
          , _current_direction (current_direction)
      {}

      WSIG(port_toplevel, port::open, ITVAL(XMLTYPE(ports_type)), port)
      {
        ui::graph::port* p (new ui::graph::port (_current_direction, NULL));
        _scene->addItem (p);

        item_by_name_type port_item_by_name;

        weaver::port wp (p, port_item_by_name);

        FROM(port) (&wp, port);
      }
    }
  }
}

