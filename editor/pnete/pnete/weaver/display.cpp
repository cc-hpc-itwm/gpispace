// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/weaver/display.hpp>

#include <pnete/data/handle/connect.hpp>
#include <pnete/data/handle/function.hpp>
#include <pnete/data/handle/net.hpp>
#include <pnete/data/handle/place.hpp>
#include <pnete/data/handle/port.hpp>
#include <pnete/data/internal.hpp>
#include <pnete/data/proxy.hpp>
#include <pnete/ui/graph/base_item.fwd.hpp>
#include <pnete/ui/graph/connectable_item.hpp>
#include <pnete/ui/graph/place.hpp>
#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/port_place_association.hpp>
#include <pnete/ui/graph/scene.hpp>
#include <pnete/ui/graph/transition.hpp>
#include <pnete/weaver/weaver.hpp>

#include <util/qt/cast.hpp>

#include <we/type/property.hpp>

#include <boost/unordered_map.hpp>

#include <QtGlobal>

namespace fhg
{
  namespace pnete
  {
    namespace weaver
    {
      namespace
      {
        typedef boost::unordered_map< std::string
                                    , ui::graph::connectable_item*
                                    > item_by_name_type;

        class function
        {
        public:
          explicit function ( const ::xml::parse::id::ref::function&
                            , data::internal_type*
                            );

          template<int Type, typename T> void weave (const T & x) {}
          template<int Type> void weave () {}

          data::proxy::type* proxy () const;

        private:
          data::proxy::type* _proxy;
          ::xml::parse::id::ref::function _function;

          ui::graph::scene_type* _scene;
          data::internal_type* _root;
        };

        class net
        {
        public:
          explicit net ( data::internal_type*
                       , ui::graph::scene_type* scene
                       , const ::xml::parse::id::ref::net& net
                       , const ::xml::parse::id::ref::function& function
                       );

          template<int Type, typename T> void weave (const T & x) {}
          template<int Type> void weave () {}

        private:
          ui::graph::scene_type* _scene;

          ::xml::parse::id::ref::net _net;
          ::xml::parse::id::ref::function _function;

          item_by_name_type _place_item_by_name;
          data::internal_type* _root;
        };

        class transition
        {
        public:
          explicit transition ( data::internal_type*
                              , ui::graph::scene_type*
                              , const ::xml::parse::id::ref::net&
                              , item_by_name_type&
                              , const ::xml::parse::id::ref::transition&
                              );

          template<int Type, typename T> void weave (const T & x) {}
          template<int Type> void weave () {}

        private:
          ui::graph::scene_type* _scene;
          ui::graph::transition_item* _transition;
          ::xml::parse::id::ref::net _net;

          item_by_name_type& _place_item_by_name;
          item_by_name_type _port_in_item_by_name;
          item_by_name_type _port_out_item_by_name;
          data::internal_type* _root;
        };

        class property
        {
        public:
          explicit property (ui::graph::base_item*);
          template<int Type, typename T> void weave (const T & x) {}
          template<int Type> void weave () {}

        private:
          ui::graph::base_item* _item;
          ::we::type::property::path_type _path;
        };

        class connection
        {
        public:
          explicit connection ( ui::graph::scene_type*
                              , item_by_name_type& place_item_by_name
                              , item_by_name_type& ports_in
                              , item_by_name_type& ports_out
                              , data::internal_type* root
                              );

          template<int Type, typename T> void weave (const T & x) {}
          template<int Type> void weave () {}

        private:
          ui::graph::scene_type* _scene;
          item_by_name_type& _place_item_by_name;
          item_by_name_type& _ports_in;
          item_by_name_type& _ports_out;
          std::string _port;
          std::string _place;
          boost::optional< ::xml::parse::id::ref::connect> _id;
          data::internal_type* _root;
        };

        class port
        {
        public:
          explicit port (ui::graph::port_item*, item_by_name_type&);

          template<int Type, typename T> void weave (const T & x) {}
          template<int Type> void weave () {}

        private:
          ui::graph::port_item* _port;

          item_by_name_type& _port_item_by_name;
        };

        class port_toplevel
        {
        public:
          explicit port_toplevel ( ui::graph::scene_type*
                                 , item_by_name_type& place_item_by_name
                                 , data::internal_type* root
                                 );

          template<int Type, typename T> void weave (const T & x) {}
          template<int Type> void weave () {}

        private:
          ui::graph::scene_type* _scene;
          item_by_name_type& _place_item_by_name;
          std::string _name;
          ui::graph::port_item* _port_item;
          data::internal_type* _root;
        };

        class place
        {
        public:
          explicit place (ui::graph::place_item*, item_by_name_type&);

          template<int Type, typename T> void weave (const T & x) {}
          template<int Type> void weave () {}

        private:
          ui::graph::place_item* _place;

          item_by_name_type& _place_item_by_name;
        };

        namespace
        {
          template<typename ID_TYPE>
          void maybe_set_position ( ui::graph::base_item* item
                                  , const ID_TYPE& id
                                  )
          {
            if (!id.get().properties().has ("fhg.pnete.position.x"))
            {
              id.get_ref().properties().set ("fhg.pnete.position.x", "0");
              item->set_just_pos_but_not_in_property (0.0, item->pos().y());
            }
            if (!id.get().properties().has ("fhg.pnete.position.y"))
            {
              id.get_ref().properties().set ("fhg.pnete.position.y", "0");
              item->set_just_pos_but_not_in_property (item->pos().x(), 0.0);
            }
          }

          template<>
          void maybe_set_position ( ui::graph::base_item* item
                                  , const ::xml::parse::id::ref::transition& id
                                  )
          {
            bool either ( !id.get().properties().has ("fhg.pnete.position.x")
                       || !id.get().properties().has ("fhg.pnete.position.y")
                        );
            if (!id.get().properties().has ("fhg.pnete.position.x"))
            {
              id.get_ref().properties().set ("fhg.pnete.position.x", "0");
              item->set_just_pos_but_not_in_property (0.0, item->pos().y());
            }
            if (!id.get().properties().has ("fhg.pnete.position.y"))
            {
              id.get_ref().properties().set ("fhg.pnete.position.y", "0");
              item->set_just_pos_but_not_in_property (item->pos().x(), 0.0);
            }
            if (either)
            {
              fhg::util::qt::throwing_qobject_cast<ui::graph::transition_item*>
                (item)->repositionChildrenAndResize();
            }
          }


        }

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
            (data::proxy::expression_proxy (data::handle::expression (id, _root)));
        }
        WSIG(function, mod::open, ::xml::parse::id::ref::module, id)
        {
          _proxy = new data::proxy::type
            (data::proxy::mod_proxy (data::handle::module (id, _root)));
        }
        WSIG(function, net::open, ::xml::parse::id::ref::net, id)
        {
          _scene = new ui::graph::scene_type
            ( data::handle::net (id, _root)
            , data::handle::function (_function, _root)
            );
          _proxy = new data::proxy::type
            (data::proxy::net_proxy (data::handle::net (id, _root), _scene));

          weaver::net wn (_root, _scene, id, _function);
          from::net (&wn, id);
        }
        WSIG(function, function::fun, XMLTYPE(function_type::content_type), fun)
        {
          from::variant (this, fun);
        }


        transition::transition ( data::internal_type* root
                               , ui::graph::scene_type* scene
                               , const ::xml::parse::id::ref::net& net
                               , item_by_name_type& place_item_by_name
                               , const ::xml::parse::id::ref::transition& id
                               )
          : _scene (scene)
          , _transition ( new ui::graph::transition_item
                          (data::handle::transition (id, root))
                        )
          , _net (net)
          , _place_item_by_name (place_item_by_name)
          , _port_in_item_by_name ()
          , _port_out_item_by_name ()
          , _root (root)
        {
          maybe_set_position (_transition, id);
          _scene->addItem (_transition);
        }

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
          from::many
            ( this
            , boost::apply_visitor (get_function (_net), fun).get().ports().ids()
            , from::port
            );
        }
        WSIG(transition, port::open, ::xml::parse::id::ref::port, port)
        {
          ui::graph::port_item* item
            ( new ui::graph::port_item
              (data::handle::port (port, _root), _transition)
            );
          maybe_set_position (item, port);
          weaver::port wp ( item
                          , port.get().direction() == we::type::PORT_IN
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

          data::handle::connect handle (*_id, _root);
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
          weaver::port_toplevel wptl (_scene, _place_item_by_name, _root);
          from::many (&wptl, _function.get().ports().ids(), from::port);
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
            (new ui::graph::place_item (data::handle::place (place, _root)));
          weaver::place wp (place_item, _place_item_by_name);
          maybe_set_position (place_item, place);
          _scene->addItem (place_item);
          from::place (&wp, place);
        }
        WSIG(net, transition::open, ::xml::parse::id::ref::transition, id)
        {
          weaver::transition wt (_root, _scene, _net, _place_item_by_name, id);
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
          , item_by_name_type& place_item_by_name
          , data::internal_type* root
          )
            : _scene (scene)
            , _place_item_by_name (place_item_by_name)
            , _name ()
            , _port_item ()
            , _root (root)
        {}

        WSIG(port_toplevel, port::open, ::xml::parse::id::ref::port, id)
        {
          _port_item = new ui::graph::top_level_port_item
            (data::handle::port (id, _root));
          maybe_set_position (_port_item, id);
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

        template<typename item_type>
          item_by_name_type name_map_for_items (const QList<item_type*>& items)
        {
          weaver::item_by_name_type result;
          foreach (item_type* item, items)
          {
            result[item->handle().get().name()] = item;
          }
          return result;
        }
      }

      namespace display
      {
        data::proxy::type function
          (const ::xml::parse::id::ref::function& fun, data::internal_type* data)
        {
          return *weaver::function (fun, data).proxy();
        }

        void transition ( const ::xml::parse::id::ref::transition& transition_id
                        , data::internal_type* root
                        , ui::graph::scene_type* scene
                        )
        {
          item_by_name_type places (name_map_for_items (scene->all_places()));
          weaver::transition wt
            ( root
            , scene
            , transition_id.get().parent()->make_reference_id()
            , places
            , transition_id
            );
          from::transition (&wt, transition_id);
        }

        void place ( const ::xml::parse::id::ref::place& place_id
                   , ui::graph::place_item* item
                   )
        {
          //! \note Does not need actual list, as it only adds itself.
          item_by_name_type place_by_name;

          weaver::place wp (item, place_by_name);
          from::place (&wp, place_id);
        }

        void top_level_port ( const ::xml::parse::id::ref::port& port_id
                            , ui::graph::scene_type* scene
                            , data::internal_type* root
                            )
        {
          item_by_name_type places (name_map_for_items (scene->all_places()));
          weaver::port_toplevel wptl (scene, places, root);
          from::port (&wptl, port_id);
        }
      }
    }
  }
}
