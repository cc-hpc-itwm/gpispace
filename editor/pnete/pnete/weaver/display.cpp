// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/weaver/display.hpp>
#include <pnete/weaver/weaver.hpp>

#include <pnete/ui/graph/scene.hpp>
#include <pnete/ui/graph/transition.hpp>
#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/place.hpp>

#include <pnete/data/internal.hpp>
#include <pnete/data/handle/net.hpp>
#include <pnete/data/handle/place.hpp>

#include <QtGlobal>

namespace fhg
{
  namespace pnete
  {
    namespace weaver
    {
      namespace visitor
      {
        class get_function
          : public boost::static_visitor<function_with_mapping_type>
        {
        private:
          XMLTYPE(net_type)& _net;

        public:
          get_function (XMLTYPE(net_type)& net) : _net (net) {}

          function_with_mapping_type
            operator() (const xml::parse::id::ref::function& fun) const
          {
            return function_with_mapping_type (fun);
          }

          function_with_mapping_type
            operator() (const xml::parse::id::ref::use& use) const
          {
            return function_with_mapping_type
              (*_net.get_function (use.get().name()));
          }
        };
      }


      function::function ( function_with_mapping_type function_with_mapping
                         , data::internal_type* root
                         )
        : _proxy (NULL)
        , _function_with_mapping (function_with_mapping)
        , _ports ()
        , _scene (NULL)
        , _root (root)
      {
        FROM (function<function> (this, _function_with_mapping.function().get_ref()));
      }
      data::proxy::type* function::proxy () const { return _proxy; }
      XMLTYPE(function_type::ports_type)& function::in ()
      {
        if (!_ports.in)
          {
            throw std::runtime_error
              ("STRANGE! function without an portlist!?");
          }

        return *_ports.in;
      }
      XMLTYPE(function_type::ports_type)& function::out ()
      {
        if (!_ports.out)
          {
            throw std::runtime_error
              ("STRANGE! function without an portlist!?");
          }

        return *_ports.out;
      }

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
      WSIG(function, mod::open, XMLTYPE(module_type), mod)
      {
        _proxy = new data::proxy::type
          ( data::proxy::mod_proxy
            ( _root
            , data::proxy::data::module_type
              ( const_cast< XMLTYPE(module_type) &> (mod)
              , in()
              , out()
              )
            , _function_with_mapping
            )
          );
      }
      WSIG(function, net::open, XMLTYPE(net_type), net)
      {
        _scene = new ui::graph::scene_type
          ( data::handle::net ( net.make_reference_id()
                              , _root->change_manager()
                              )
          , _root
          );
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
                       , _function_with_mapping.function()
                       );

        FROM(net) (&wn, net);
      }
      WSIG(function, function::in, XMLTYPE(function_type::ports_type), in)
      {
        _ports.in = const_cast<XMLTYPE(function_type::ports_type)*> (&in);
      }
      WSIG(function, function::out, XMLTYPE(function_type::ports_type), out)
      {
        _ports.out = const_cast<XMLTYPE(function_type::ports_type)*> (&out);
      }
      WSIG(function, function::fun, XMLTYPE(function_type::type), fun)
      {
        boost::apply_visitor (FROM(visitor::deref_variant<function>) (this), fun);
      }


      transition::transition ( data::internal_type* root
                             , ui::graph::scene_type* scene
                             , ui::graph::transition_item* transition
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
        , _function (boost::none)
      {}
      function_with_mapping_type
      transition::get_function (XMLTYPE(transition_type::function_or_use_type)& f)
      {
        return boost::apply_visitor (visitor::get_function(_net), f);
      }

      WSIG( transition
          , transition::function
          , XMLTYPE(transition_type::function_or_use_type)
          , fun
          )
      {
        function_with_mapping_type function_with_mapping
          (get_function (const_cast<XMLTYPE(transition_type::function_or_use_type)&>(fun)));

        weaver::function sub (function_with_mapping, _root);

        _type_map = function_with_mapping.type_map();
        _function = function_with_mapping.function();

        _transition->set_proxy (sub.proxy());

        _current_port_direction = ui::graph::connectable::direction::IN;
        from::many (this, sub.in().values(), FROM(port));

        _current_port_direction = ui::graph::connectable::direction::OUT;
        from::many (this, sub.out().values(), FROM(port));
      }
      WSIG(transition, port::open, XMLTYPE(port_type), port)
      {
        ui::graph::port_item* port_item
          ( new ui::graph::port_item
          ( const_cast<XMLTYPE(port_type)&> (port)
            , _current_port_direction
            , _type_map
            , _transition
            )
          );

        weaver::port wp ( port_item
                        , _current_port_direction == ui::graph::connectable::direction::IN
                        ? _port_in_item_by_name
                        : _port_out_item_by_name
                        );

        FROM(port) (&wp, port);
      }
      WSIGE(transition, transition::close)
      {
        //! \todo do something if not already set
        //        _transition->repositionChildrenAndResize();
      }
      WSIG(transition, transition::connect_read, XMLTYPE(transition_type::connections_type), cs)
      {
        weaver::connection wc ( _scene
                              , _place_item_by_name
                              , _port_in_item_by_name
                              , ui::graph::connectable::direction::IN
                              , true
                              );

        from::many (&wc, cs.values(), FROM(connection));
      }
      WSIG(transition, transition::connect_in, XMLTYPE(transition_type::connections_type), cs)
      {
        weaver::connection wc ( _scene
                              , _place_item_by_name
                              , _port_in_item_by_name
                              , ui::graph::connectable::direction::IN
                              );

        from::many (&wc, cs.values(), FROM(connection));
      }
      WSIG(transition, transition::connect_out, XMLTYPE(transition_type::connections_type), cs)
      {
        weaver::connection wc ( _scene
                              , _place_item_by_name
                              , _port_out_item_by_name
                              , ui::graph::connectable::direction::OUT
                              );

        from::many (&wc, cs.values(), FROM(connection));
      }
      WSIG(transition, transition::properties, WETYPE(property::type), props)
      {
        weaver::property wp (_transition);

        FROM (properties) (&wp, props);
      }

      property::property (ui::graph::base_item* item)
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
            if (_path.size() > 2 && _path[2] == "orientation")
              {
                _item->set_just_orientation_but_not_in_property
                  (ui::graph::port::orientation::read (value));
              }
            if (_path.size() > 2 && _path[2] == "position")
              {
                if (_path.size() > 3)
                  {
                    if (_path[3] == "x")
                      {
                        _item->set_just_pos_but_not_in_property
                          ( boost::lexical_cast<qreal>(value)
                          , _item->pos().y()
                          );
                      }
                    else if (_path[3] == "y")
                      {
                        _item->set_just_pos_but_not_in_property
                          ( _item->pos().x()
                          , boost::lexical_cast<qreal>(value)
                          );
                      }
                  }
              }
          }
      }

      connection::connection ( ui::graph::scene_type* scene
                             , item_by_name_type& place_item_by_name
                             , item_by_name_type& port_item_by_name
                             , const ui::graph::connectable::direction::type& direction
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

        if (_direction == ui::graph::connectable::direction::IN)
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
               , ui::graph::scene_type* scene
               , XMLTYPE(net_type)& net
               , XMLTYPE(function_type::ports_type)& in
               , XMLTYPE(function_type::ports_type)& out
               , XMLTYPE(function_type)& function
               )
        : _scene (scene)
        , _net (net)
        , _in (in)
        , _out (out)
        , _function (function)
        , _place_item_by_name ()
        , _root (root)
      {}
      WSIGE(net, net::close)
      {
        {
          weaver::port_toplevel wptl ( _scene
                                     , ui::graph::connectable::direction::OUT
                                     , _place_item_by_name
                                     , _root
                                     , _function
                                     );
          from::many (&wptl, _in.values(), FROM(port));
        }

        {
          weaver::port_toplevel wptl ( _scene
                                     , ui::graph::connectable::direction::IN
                                     , _place_item_by_name
                                     , _root
                                     , _function
                                     );
          from::many (&wptl, _out.values(), FROM(port));
        }
      }
      WSIG(net, net::transitions, XMLTYPE(net_type::transitions_type), transitions)
      {
        from::many (this, transitions.values(), FROM(transition));
      }
      WSIG(net, net::places, XMLTYPE(net_type::places_type), places)
      {
        from::many (this, places.values(), FROM(place));
      }
      WSIG(net, place::open, XMLTYPE(place_type), place)
      {
        ui::graph::place_item* place_item
          ( new ui::graph::place_item
            ( data::handle::place ( place.make_reference_id()
                                  , _root->change_manager()
                                  )
            )
          );
        weaver::place wp (place_item, _place_item_by_name);
        _scene->addItem (place_item);
        FROM(place) (&wp, place);
      }
      WSIG(net, transition::open, XMLTYPE(transition_type), transition)
      {
        ui::graph::transition_item* trans
          ( new ui::graph::transition_item
            ( data::handle::transition ( transition.make_reference_id()
                                       , _root->change_manager()
                                       )
            )
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


      port::port ( ui::graph::port_item* port
                 , item_by_name_type& port_item_by_name
                 )
        : _port (port)
        , _port_item_by_name (port_item_by_name)
      {}

      WSIG(port, port::name, std::string, name)
      {
        _port_item_by_name[name] = _port;
      }
      WSIG(port, port::properties, WETYPE(property::type), props)
      {
        weaver::property wp (_port);

        FROM (properties) (&wp, props);
      }

      place::place ( ui::graph::place_item* place
                   , item_by_name_type& place_item_by_name
                   )
        : _place (place)
        , _place_item_by_name (place_item_by_name)
      {}

      WSIG(place, place::name, std::string, name)
      {
        _place_item_by_name[name] = _place;
      }
      WSIG(place, place::properties, WETYPE(property::type), props)
      {
        weaver::property wp (_place);

        FROM(properties) (&wp, props);
      }

      port_toplevel::port_toplevel
        ( ui::graph::scene_type* scene
        , const ui::graph::connectable::direction::type& direction
        , item_by_name_type& place_item_by_name
        , data::internal_type* root
        , XMLTYPE(function_type)& function
        )
          : _scene (scene)
          , _place_item_by_name (place_item_by_name)
          , _name ()
          , _direction (direction)
          , _port_item ()
          , _root (root)
          , _function (function)
      {}

      WSIG(port_toplevel, port::open, XMLTYPE(port_type), port)
      {
        _port_item = new ui::graph::top_level_port_item
          ( const_cast<XMLTYPE(port_type)&> (port)
          , _direction
          );
        _scene->addItem (_port_item);
      }
      WSIG(port_toplevel, port::name, std::string, name)
      {
        _name = name;
      }
      WSIG(port_toplevel, port::place, MAYBE(std::string), place)
      {
        if (place)
        {
          item_by_name_type _port_item_by_name;

          _port_item_by_name[_name] = _port_item;

          typedef item_by_name_type::iterator iterator_type;

          const iterator_type port_pos (_port_item_by_name.find (_name));
          const iterator_type place_pos (_place_item_by_name.find (*place));

          if (port_pos == _port_item_by_name.end())
          {
            throw
              std::runtime_error ("connection: port " + _name + " not found");
          }
          if (place_pos == _place_item_by_name.end())
          {
            throw
              std::runtime_error ("connection: place " + *place + " not found");
          }

          //! \todo Not connection, but association!
          if (_direction == ui::graph::connectable::direction::IN)
          {
            _scene->create_connection ( place_pos->second
                                      , port_pos->second
                                      , false
                                      );
          }
          else
          {
            _scene->create_connection ( port_pos->second
                                      , place_pos->second
                                      , false
                                      );
          }
        }
      }
      WSIG(port_toplevel, port::properties, WETYPE(property::type), props)
      {
        weaver::property wp (_port_item);

        FROM (properties) (&wp, props);
      }
    }
  }
}
