// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/traverse/display.hpp>
#include <pnete/traverse/weaver.hpp>

#include <pnete/ui/GraphScene.hpp>
#include <pnete/ui/GraphTransition.hpp>

#include <xml/parse/types.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace weaver
    {
      namespace display
      {
        weaver::weaver (::xml::parse::type::function_type & fun)
          : _proxy (NULL)
          , _fun (fun)
          , _scene (NULL)
        {
          FROM (function<weaver> (this, fun));
        }

        data::proxy::type* weaver::proxy () const { return _proxy; }

        WSIG(expression::open, XMLTYPE(expression_type), exp)
        {
          _proxy = new data::proxy::type
            (data::proxy::expression_proxy
              (const_cast< XMLTYPE(expression_type) &> (exp), _fun)
            );
        }

        WSIG(mod::open, XMLTYPE(mod_type), mod)
        {
          _proxy = new data::proxy::type
            (data::proxy::mod_proxy
              (const_cast< XMLTYPE(mod_type) &> (mod), _fun)
            );
        }

        WSIG(net::open, XMLTYPE(net_type), net)
        {
          _scene = new graph::Scene(const_cast< XMLTYPE(net_type) &> (net));
          _proxy = new data::proxy::type
            (data::proxy::net_proxy
            (const_cast< XMLTYPE(net_type) &> (net), _fun, _scene)
            );
        }

        WSIG(function::fun, XMLTYPE(function_type::type), fun)
        {
          boost::apply_visitor (FROM(visitor::net_type<weaver>) (this), fun);
        }

        /*
        WSIG( transition::open
            , ITVAL(XMLTYPE(net_type::transitions_type))
            , transition
            )
        {
          _current = new graph::Transition
            (const_cast<graph::Transition::transition_type &> (transition));

          _scene->addItem (_current);
        }
        WSIGE(transition::close)
        {
          _current->repositionChildrenAndResize();
        }
        WSIG(transition::name, std::string, name)
        {
          _current->name (QString(name.c_str()));
        }
        WSIG(transition::function,  XMLTYPE(transition_type::f_type), fun)
        {
          boost::apply_visitor
            (FROM(visitor::function_type<weaver>) (this), fun);
        }

        WSIG(function::open, ITVAL(XMLTYPE(net_type::functions_type)), fun)
        {
          //         graph::Scene* subnet (scene_producer (fun, _current));
          //         if (subnet)
          //           {
          //             _current->subnet (subnet);
          //           }
        }

        */
      }
    }
  }
}
