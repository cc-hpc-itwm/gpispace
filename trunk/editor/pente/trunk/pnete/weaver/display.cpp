// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/weaver/display.hpp>
#include <pnete/weaver/weaver.hpp>

#include <pnete/ui/GraphScene.hpp>
#include <pnete/ui/GraphTransition.hpp>
#include <pnete/ui/GraphPort.hpp>

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

      transition::transition (graph::Transition* transition)
        : _transition (transition)
        , _current_port_direction ()
      {}
      XMLTYPE(function_type)& transition::get_function
      (const XMLTYPE(transition_type::f_type) & f)
      {
        return boost::apply_visitor (visitor::get_function(), f);
      }

      port::port (graph::Port* port) : _port (port) {}

      port_toplevel::port_toplevel
        ( graph::Scene* const scene
        , const graph::Port::eDirection& current_direction
        )
          : _scene (scene)
          , _current_direction (current_direction)
      {}

      place::place (ui::graph::place* place) : _place (place) {}

      net::net ( graph::Scene* scene
               , XMLTYPE(net_type)& net
               , XMLTYPE(ports_type)& in
               , XMLTYPE(ports_type)& out
               )
        : _scene (scene)
      {
        {
          weaver::port_toplevel wptl (_scene, graph::Port::OUT);
          from::many (&wptl, in, FROM(port));
        }

        {
          weaver::port_toplevel wptl (_scene, graph::Port::IN);
          from::many (&wptl, out, FROM(port));
        }
      }

      namespace visitor
      {
        function_type& default_fun()
        {
          static function_type f;

          return f;
        }
      }

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
        _scene = new graph::Scene(const_cast< XMLTYPE(net_type) &> (net));
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
      WSIG(net, net::transitions, XMLTYPE(net_type::transitions_type), transitions)
      {
        typedef XMLTYPE(net_type::transitions_type)::const_iterator iterator;

        for ( iterator trans (transitions.begin()), end (transitions.end())
                ; trans != end
                ; ++trans
            )
          {
            FROM(transition) (this, *trans);
          }
      }
      WSIG(net, net::places, XMLTYPE(net_type::places_type), places)
      {
        from::many (this, places, FROM(place));
      }

      WSIG(net, place::open, ITVAL(XMLTYPE(net_type::places_type)), place)
      {
        ui::graph::place* place_item (new ui::graph::place());
        weaver::place wp (place_item);
        _scene->addItem (place_item);
        FROM(place) (&wp, place);
      }
      WSIG(place, place::name, std::string, name)
      {
        _place->name (QString (name.c_str()));
      }
      WSIG(place, place::type, std::string, type)
      {
        _place->we_type (QString (type.c_str()));
      }

      WSIG( net
          , transition::open
          , ITVAL(XMLTYPE(net_type::transitions_type))
          , transition
          )
      {
        graph::Transition* trans
          ( new graph::Transition
            (const_cast<graph::Transition::transition_type &> (transition))
          );
        _scene->addItem (trans);
        weaver::transition wt (trans);
        FROM(transition) (&wt, transition);
      }
      WSIGE(transition, transition::close)
      {
        _transition->repositionChildrenAndResize();
      }
      WSIG(transition, transition::name, std::string, name)
      {
        _transition->name (QString(name.c_str()));
      }
      WSIG(transition, transition::function, XMLTYPE(transition_type::f_type), fun)
      {
        function sub (get_function (fun));

        _transition->proxy (sub.proxy());

        _current_port_direction = graph::Port::IN;
        from::many (this, sub.in(), FROM(port));

        _current_port_direction = graph::Port::OUT;
        from::many (this, sub.out(), FROM(port));
      }

      WSIG(transition, port::open, ITVAL(XMLTYPE(ports_type)), port)
      {
        weaver::port wp (new graph::Port (_transition, _current_port_direction));

        FROM(port) (&wp, port);
      }

      WSIG(port, port::name, std::string, name)
      {
        _port->name (QString (name.c_str()));
      }
      WSIG(port, port::type, std::string, type)
      {
        _port->we_type (QString (type.c_str()));
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

      WSIG(port_toplevel, port::open, ITVAL(XMLTYPE(ports_type)), port)
      {
        graph::Port* p (new graph::Port (NULL, _current_direction));
        _scene->addItem (p);
        weaver::port wp (p);

        FROM(port) (&wp, port);
      }
    }
  }
}

