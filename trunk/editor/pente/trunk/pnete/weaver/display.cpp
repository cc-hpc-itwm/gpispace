// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/weaver/display.hpp>
#include <pnete/weaver/weaver.hpp>

#include <pnete/ui/GraphScene.hpp>
#include <pnete/ui/GraphTransition.hpp>
#include <pnete/ui/GraphPort.hpp>

#include <xml/parse/types.hpp>

#include <iostream>

namespace fhg
{
  namespace pnete
  {
    namespace weaver
    {
      display::display (XMLTYPE(function_type) & fun)
        : _proxy (NULL)
        , _fun (fun)
        , _scene (NULL)
        , _current (NULL)
      {
        FROM (function<display> (this, fun));
      }

      data::proxy::type* display::proxy () const { return _proxy; }

      XMLTYPE(function_type)& display::get_function
      (const XMLTYPE(transition_type::f_type) & f)
      {
        return boost::apply_visitor (visitor::get_function(), f);
      }

      namespace visitor
      {
        function_type& default_fun()
        {
          static function_type f;

          return f;
        }
      }

      WSIG(display, expression::open, XMLTYPE(expression_type), exp)
      {
        _proxy = new data::proxy::type
          (data::proxy::expression_proxy
          (const_cast< XMLTYPE(expression_type) &> (exp), _fun)
          );
      }

      WSIG(display, mod::open, XMLTYPE(mod_type), mod)
      {
        _proxy = new data::proxy::type
          (data::proxy::mod_proxy
          (const_cast< XMLTYPE(mod_type) &> (mod), _fun)
          );
      }

      WSIG(display, net::open, XMLTYPE(net_type), net)
      {
        _scene = new graph::Scene(const_cast< XMLTYPE(net_type) &> (net));
        _proxy = new data::proxy::type
          (data::proxy::net_proxy
          (const_cast< XMLTYPE(net_type) &> (net), _fun, _scene)
          );
      }
      WSIG(display, net::transitions, XMLTYPE(net_type::transitions_type), transitions)
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

      WSIG(display,  transition::open
          , ITVAL(XMLTYPE(net_type::transitions_type))
          , transition
          )
      {
        _current = new graph::Transition
          (const_cast<graph::Transition::transition_type &> (transition));

        _scene->addItem (_current);
      }
      WSIGE(display, transition::close)
      {
        _current->repositionChildrenAndResize();
      }
      WSIG(display, transition::name, std::string, name)
      {
        _current->name (QString(name.c_str()));
      }
      WSIG(display, transition::function, XMLTYPE(transition_type::f_type), fun)
      {
        display sub (get_function (fun));

        _current->proxy (sub.proxy());

        typedef XMLTYPE(ports_type)::const_iterator iterator;

        //! \todo references to port inside graph::Port
        for ( iterator port (sub._function_state.in.begin())
                , end (sub._function_state.in.end())
                ; port != end
                ; ++port
            )
          {
            graph::Port* p (new graph::Port (_current, graph::Port::IN));

            p->name (QString (port->name.c_str()));
            p->we_type (QString (port->type.c_str()));
          }

        for ( iterator port (sub._function_state.out.begin())
                , end (sub._function_state.out.end())
                ; port != end
                ; ++port
            )
          {
            new graph::Port (_current, graph::Port::OUT);
          }
      }

      WSIG(display, function::in, XMLTYPE(ports_type), in)
      {
        _function_state.in = in;
      }
      WSIG(display, function::out, XMLTYPE(ports_type), out)
      {
        _function_state.out = out;
      }
      WSIG(display, function::fun, XMLTYPE(function_type::type), fun)
      {
        boost::apply_visitor (FROM(visitor::net_type<display>) (this), fun);
      }
    }
  }
}

