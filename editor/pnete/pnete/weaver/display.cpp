// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/weaver/display.hpp>
#include <pnete/weaver/weaver.hpp>

#include <pnete/data/handle/connect.hpp>
#include <pnete/data/handle/function.hpp>
#include <pnete/data/handle/net.hpp>
#include <pnete/data/handle/place.hpp>
#include <pnete/data/handle/port.hpp>
#include <pnete/data/internal.hpp>
#include <pnete/ui/graph/place.hpp>
#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/port_place_association.hpp>
#include <pnete/ui/graph/scene.hpp>
#include <pnete/ui/graph/transition.hpp>

#include <QtGlobal>

namespace fhg
{
  namespace pnete
  {
    namespace weaver
    {
      function::function ( const ::xml::parse::id::ref::function& function
                         , data::internal_type* root
                         )
        : _proxy (NULL)
        , _function (function)
        , _scene (NULL)
        , _root (root)
      {
        from::function (this, _function);
      }
      data::proxy::type* function::proxy () const { return _proxy; }

      WSIG(function, expression::open, ::xml::parse::id::ref::expression, id)
      {
        _proxy = new data::proxy::type
          ( data::proxy::expression_proxy
            ( _root
            , data::handle::function (_function, _root->change_manager())
            , id
            )
          );
      }
      WSIG(function, mod::open, ::xml::parse::id::ref::module, id)
      {
        _proxy = new data::proxy::type
          ( data::proxy::mod_proxy
            ( _root
            , data::handle::function (_function, _root->change_manager())
            , id
            )
          );
      }
      WSIG(function, net::open, ::xml::parse::id::ref::net, id)
      {
        _scene = new ui::graph::scene_type
          ( data::handle::net (id, _root->change_manager())
          , data::handle::function (_function, _root->change_manager())
          , _root
          );
        _proxy = new data::proxy::type
          ( data::proxy::net_proxy
            ( _root
            , data::handle::function (_function, _root->change_manager())
            , id
            , _scene
            )
          );

        weaver::net wn (_root, _scene, id, _function);
        from::net (&wn, id);
      }
      WSIG(function, function::fun, XMLTYPE(function_type::type), fun)
      {
        from::variant (this, fun);
      }


      transition::transition ( data::internal_type* root
                             , ui::graph::scene_type* scene
                             , ui::graph::transition_item* transition
                             , const ::xml::parse::id::ref::net& net
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
      {}

      namespace
      {
        class get_function
          : public boost::static_visitor< ::xml::parse::id::ref::function>
        {
        private:
          ::xml::parse::id::ref::net _net;

        public:
          get_function (const ::xml::parse::id::ref::net& id) : _net (id) {}

          ::xml::parse::id::ref::function
            operator() (const xml::parse::id::ref::function& fun) const
          {
            return fun;
          }

          ::xml::parse::id::ref::function
            operator() (const xml::parse::id::ref::use& use) const
          {
            return *_net.get().get_function (use.get().name());
          }
        };
      }

      WSIG( transition
          , transition::function
          , ::xml::parse::type::transition_type::function_or_use_type
          , fun
          )
      {
        const ::xml::parse::id::ref::function function
          (boost::apply_visitor (get_function (_net), fun));

        weaver::function sub (function, _root);

        _transition->set_proxy (sub.proxy());

        _current_port_direction = ui::graph::connectable::direction::IN;
        from::many (this, function.get().in().ids(), from::port);

        _current_port_direction = ui::graph::connectable::direction::OUT;
        from::many (this, function.get().out().ids(), from::port);
      }
      WSIG(transition, port::open, ::xml::parse::id::ref::port, port)
      {
        ui::graph::port_item* port_item
          ( new ui::graph::port_item
            ( data::handle::port (port, _root->change_manager())
            , _current_port_direction
            , _transition
            )
          );

        weaver::port wp ( port_item
                        , _current_port_direction == ui::graph::connectable::direction::IN
                        ? _port_in_item_by_name
                        : _port_out_item_by_name
                        );

        from::port (&wp, port);
      }
      WSIGE(transition, transition::close)
      {
        //! \todo do something if not already set
        //        _transition->repositionChildrenAndResize();
      }
      WSIG(transition, transition::connection, XMLTYPE(transition_type::connections_type), cs)
      {
        weaver::connection wc ( _scene
                              , _place_item_by_name
                              , _port_in_item_by_name
                              , _port_out_item_by_name
                              , _root
                              );

        from::many (&wc, cs.ids(), from::connection);
      }
      WSIG(transition, transition::properties, WETYPE(property::type), props)
      {
        weaver::property wp (_transition);

        from::properties (&wp, props);
      }

      property::property (ui::graph::base_item* item)
        : _item (item)
        , _path ()
      {}
      WSIG(property, properties::open, WETYPE(property::type), props)
      {
        from::many (this, props.get_map(), from::property);
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
                             , item_by_name_type& ports_in
                             , item_by_name_type& ports_out
                             , data::internal_type* root
                             )
        : _scene (scene)
        , _place_item_by_name (place_item_by_name)
        , _ports_in (ports_in)
        , _ports_out (ports_out)
        , _port ()
        , _place ()
        , _id (boost::none)
        , _root (root)
      {}

      WSIG (connection, connection::open, ::xml::parse::id::ref::connect, id)
      {
        _id = id;
      }

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
        const bool is_out (!petri_net::edge::is_PT (_id->get().direction()));

        typedef item_by_name_type::iterator iterator_type;

        const iterator_type port_pos
          ((is_out ? _ports_out : _ports_in).find (_port));
        if (port_pos == (is_out ? _ports_out : _ports_in).end())
        {
          throw std::runtime_error ("connection: port " + _port + " not found");
        }

        const iterator_type place_pos (_place_item_by_name.find (_place));
        if (place_pos == _place_item_by_name.end())
        {
          throw
            std::runtime_error ("connection: place " + _place + " not found");
        }

        data::handle::connect handle (*_id, _root->change_manager());
        if (!is_out)
        {
          _scene->create_connection
            (place_pos->second, port_pos->second, handle);
        }
        else
        {
          _scene->create_connection
            (port_pos->second, place_pos->second, handle);
        }
      }

      net::net ( data::internal_type* root
               , ui::graph::scene_type* scene
               , const ::xml::parse::id::ref::net& net
               , const ::xml::parse::id::ref::function& function
               )
        : _scene (scene)
        , _net (net)
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
          from::many (&wptl, _function.get().in().ids(), from::port);
        }

        {
          weaver::port_toplevel wptl ( _scene
                                     , ui::graph::connectable::direction::IN
                                     , _place_item_by_name
                                     , _root
                                     , _function
                                     );
          from::many (&wptl, _function.get().out().ids(), from::port);
        }
      }
      WSIG(net, net::transitions, XMLTYPE(net_type::transitions_type), transitions)
      {
        from::many (this, transitions.ids(), from::transition);
      }
      WSIG(net, net::places, XMLTYPE(net_type::places_type), places)
      {
        from::many (this, places.ids(), from::place);
      }
      WSIG(net, place::open, ::xml::parse::id::ref::place, place)
      {
        ui::graph::place_item* place_item
          ( new ui::graph::place_item
            (data::handle::place (place, _root->change_manager()))
          );
        weaver::place wp (place_item, _place_item_by_name);
        _scene->addItem (place_item);
        from::place (&wp, place);
      }
      WSIG(net, transition::open, ::xml::parse::id::ref::transition, id)
      {
        ui::graph::transition_item* trans
          ( new ui::graph::transition_item
            (data::handle::transition (id, _root->change_manager()))
          );
        _scene->addItem (trans);
        weaver::transition wt ( _root
                              , _scene
                              , trans
                              , _net
                              , _place_item_by_name
                              );
        from::transition (&wt, id);
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

        from::properties (&wp, props);
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

        from::properties (&wp, props);
      }

      port_toplevel::port_toplevel
        ( ui::graph::scene_type* scene
        , const ui::graph::connectable::direction::type& direction
        , item_by_name_type& place_item_by_name
        , data::internal_type* root
        , const ::xml::parse::id::ref::function& function
        )
          : _scene (scene)
          , _place_item_by_name (place_item_by_name)
          , _name ()
          , _direction (direction)
          , _port_item ()
          , _root (root)
          , _function (function)
      {}

      WSIG(port_toplevel, port::open, ::xml::parse::id::ref::port, id)
      {
        _port_item = new ui::graph::top_level_port_item
          (data::handle::port (id, _root->change_manager()), _direction);
        _scene->addItem (_port_item);
      }
      WSIG(port_toplevel, port::name, std::string, name)
      {
        _name = name;
      }
      WSIG(port_toplevel, port::place, boost::optional<std::string>, place)
      {
        if (place)
        {
          const item_by_name_type::iterator place_pos
            (_place_item_by_name.find (*place));

          if (place_pos == _place_item_by_name.end())
          {
            throw
              std::runtime_error ("connection: place " + *place + " not found");
          }

          _scene->addItem
            ( new ui::graph::port_place_association
              ( _port_item
              , qgraphicsitem_cast<ui::graph::place_item*> (place_pos->second)
              , _port_item->handle()
              )
            );
        }
      }
      WSIG(port_toplevel, port::properties, WETYPE(property::type), props)
      {
        weaver::property wp (_port_item);

        from::properties (&wp, props);
      }
    }
  }
}
