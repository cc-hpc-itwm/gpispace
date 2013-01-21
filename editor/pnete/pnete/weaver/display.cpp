// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/weaver/display.hpp>

#include <pnete/data/handle/connect.hpp>
#include <pnete/data/handle/function.hpp>
#include <pnete/data/handle/net.hpp>
#include <pnete/data/handle/place.hpp>
#include <pnete/data/handle/port.hpp>
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

        template<typename ID_TYPE>
          void initialize_and_set_position ( ui::graph::base_item* item
                                           , const ID_TYPE& id
                                           )
        {
          if (!id.get().properties().has ("fhg.pnete.position.x"))
          {
            id.get_ref().properties().set ("fhg.pnete.position.x", "0.0");
          }
          if (!id.get().properties().has ("fhg.pnete.position.y"))
          {
            id.get_ref().properties().set ("fhg.pnete.position.y", "0.0");
          }

          item->set_just_pos_but_not_in_property
            ( boost::lexical_cast<qreal>
              (id.get().properties().get ("fhg.pnete.position.x"))
            , boost::lexical_cast<qreal>
              (id.get().properties().get ("fhg.pnete.position.y"))
            );
        }

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
          }
        }

        class port
        {
        public:
          explicit port ( ui::graph::port_item* port
                        , item_by_name_type& port_item_by_name
                        )
            : _port (port)
            , _port_item_by_name (port_item_by_name)
          { }

          template<int Type, typename T> void weave (const T & x) {}
          template<int Type> void weave () {}

        private:
          ui::graph::port_item* _port;

          item_by_name_type& _port_item_by_name;
        };

        WSIG (port, port::open, xml::parse::id::ref::port, id)
        {
          const ::xml::parse::type::port_type& port (id.get());
          _port_item_by_name[port.name()] = _port;

          weaver::property wp (_port);
          from::properties (&wp, port.properties());
        }

        class connection
        {
        public:
          explicit connection ( ui::graph::scene_type* scene
                              , item_by_name_type& place_item_by_name
                              , item_by_name_type& ports_in
                              , item_by_name_type& ports_out
                              , data::internal_type* root
                              )
            : _scene (scene)
            , _place_item_by_name (place_item_by_name)
            , _ports_in (ports_in)
            , _ports_out (ports_out)
            , _root (root)
          { }

          template<int Type, typename T> void weave (const T & x) {}
          template<int Type> void weave () {}

        private:
          ui::graph::scene_type* _scene;
          item_by_name_type& _place_item_by_name;
          item_by_name_type& _ports_in;
          item_by_name_type& _ports_out;
          data::internal_type* _root;
        };

        WSIG (connection, connection::open, ::xml::parse::id::ref::connect, id)
        {
          const ::xml::parse::type::connect_type& connection (id.get());

          const bool is_out (!petri_net::edge::is_PT (connection.direction()));

          typedef item_by_name_type::iterator iterator_type;

          const iterator_type port_pos
            ((is_out ? _ports_out : _ports_in).find (connection.port()));
          if (port_pos == (is_out ? _ports_out : _ports_in).end())
          {
            throw std::runtime_error ("connection: port " + connection.port() + " not found");
          }

          const iterator_type place_pos (_place_item_by_name.find (connection.place()));
          if (place_pos == _place_item_by_name.end())
          {
            throw
              std::runtime_error ("connection: place " + connection.place() + " not found");
          }

          data::handle::connect handle (id, _root);
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

        class transition
        {
        public:
          explicit transition ( data::internal_type* root
                              , ui::graph::scene_type* scene
                              , item_by_name_type& place_item_by_name
                              , const ::xml::parse::id::ref::transition& id
                              )
            : _scene (scene)
            , _transition ( new ui::graph::transition_item
                            (data::handle::transition (id, root))
                          )
            , _place_item_by_name (place_item_by_name)
            , _port_in_item_by_name ()
            , _port_out_item_by_name ()
            , _root (root)
          {
            initialize_and_set_position (_transition, id);
            _scene->addItem (_transition);
          }

          template<int Type, typename T> void weave (const T & x) {}
          template<int Type> void weave () {}

        private:
          ui::graph::scene_type* _scene;
          ui::graph::transition_item* _transition;

          item_by_name_type& _place_item_by_name;
          item_by_name_type _port_in_item_by_name;
          item_by_name_type _port_out_item_by_name;
          data::internal_type* _root;
        };

        WSIG (transition, transition::open, xml::parse::id::ref::transition, id)
        {
          const ::xml::parse::type::transition_type& trans (id.get());
          weaver::property wp (_transition);

          from::properties (&wp, trans.properties());

          from::many
            ( this
            , trans.resolved_function().get().ports().ids()
            , from::port
            );

          weaver::connection wc ( _scene
                                , _place_item_by_name
                                , _port_in_item_by_name
                                , _port_out_item_by_name
                                , _root
                                );

          from::many (&wc, trans.connections().ids(), from::connection);

          //! \todo do something if not already set
          //        _transition->repositionChildrenAndResize();

        }

        WSIG (transition, port::open, ::xml::parse::id::ref::port, port)
        {
          ui::graph::port_item* item
            ( new ui::graph::port_item
              (data::handle::port (port, _root), _transition)
            );
          //! \todo This sets the wrong position: differentiate
          //! between ports on transition and ports in net (inner, outer)
          initialize_and_set_position (item, port);
          weaver::port wp ( item
                          , port.get().direction() == we::type::PORT_IN
                          ? _port_in_item_by_name
                          : _port_out_item_by_name
                          );

          from::port (&wp, port);
        }

        class port_toplevel
        {
        public:
          explicit port_toplevel ( ui::graph::scene_type* scene
                                 , item_by_name_type& place_item_by_name
                                 , data::internal_type* root
                                 )
            : _scene (scene)
            , _place_item_by_name (place_item_by_name)
            , _port_item ()
            , _root (root)
          { }

          template<int Type, typename T> void weave (const T & x) {}
          template<int Type> void weave () {}

        private:
          ui::graph::scene_type* _scene;
          item_by_name_type& _place_item_by_name;
          ui::graph::port_item* _port_item;
          data::internal_type* _root;
        };

        WSIG (port_toplevel, port::open, ::xml::parse::id::ref::port, id)
        {
          const ::xml::parse::type::port_type& port (id.get());

          _port_item = new ui::graph::top_level_port_item
            (data::handle::port (id, _root));
          initialize_and_set_position (_port_item, id);
          _scene->addItem (_port_item);

          if (port.place)
          {
            const item_by_name_type::iterator place_pos
              (_place_item_by_name.find (*port.place));

            if (place_pos == _place_item_by_name.end())
            {
              throw
                std::runtime_error ("connection: place " + *port.place + " not found");
            }

            _scene->addItem
              ( new ui::graph::port_place_association
                ( _port_item
                , qgraphicsitem_cast<ui::graph::place_item*> (place_pos->second)
                , _port_item->handle()
                )
              );
          }

          weaver::property wp (_port_item);

          from::properties (&wp, port.properties());
        }

        class place
        {
        public:
          explicit place ( ui::graph::place_item* place
                         , item_by_name_type& place_item_by_name
                         )
            : _place (place)
            , _place_item_by_name (place_item_by_name)
          { }

          template<int Type, typename T> void weave (const T & x) {}
          template<int Type> void weave () {}

        private:
          ui::graph::place_item* _place;

          item_by_name_type& _place_item_by_name;
        };

        WSIG (place, place::open, ::xml::parse::id::ref::place, id)
        {
          const ::xml::parse::type::place_type& place (id.get());

          _place_item_by_name[place.name()] = _place;

          weaver::property wp (_place);

          from::properties (&wp, place.properties());
        }

        class net
        {
        public:
          explicit net ( data::internal_type* root
                       , ui::graph::scene_type* scene
                       , const ::xml::parse::id::ref::net& net
                       , const ::xml::parse::id::ref::function& function
                       )
            : _scene (scene)
            , _net (net)
            , _function (function)
            , _place_item_by_name ()
            , _root (root)
          { }

          template<int Type, typename T> void weave (const T & x) {}
          template<int Type> void weave () {}

        private:
          ui::graph::scene_type* _scene;

          ::xml::parse::id::ref::net _net;
          ::xml::parse::id::ref::function _function;

          item_by_name_type _place_item_by_name;
          data::internal_type* _root;
        };

        WSIG (net, net::open, ::xml::parse::id::ref::net, id)
        {
          const ::xml::parse::type::net_type& net (id.get());
          from::many (this, net.places().ids(), from::place);
          from::many (this, net.transitions().ids(), from::transition);

          weaver::port_toplevel wptl (_scene, _place_item_by_name, _root);
          from::many (&wptl, _function.get().ports().ids(), from::port);
        }

        WSIG (net, transition::open, ::xml::parse::id::ref::transition, id)
        {
          weaver::transition wt (_root, _scene, _place_item_by_name, id);
          from::transition (&wt, id);
        }

        WSIG (net, place::open, ::xml::parse::id::ref::place, place)
        {
          ui::graph::place_item* place_item
            (new ui::graph::place_item (data::handle::place (place, _root)));
          weaver::place wp (place_item, _place_item_by_name);
          initialize_and_set_position (place_item, place);
          _scene->addItem (place_item);
          from::place (&wp, place);
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
        ui::graph::scene_type* net ( const data::handle::net& net
                                   , const data::handle::function& parent
                                   )
        {
          ui::graph::scene_type* scene (new ui::graph::scene_type (net, parent));

          weaver::net wn (net.document(), scene, net.id(), parent.id());
          from::net (&wn, net.id());

          return scene;
        }

        void transition ( const data::handle::transition& transition
                        , ui::graph::scene_type* scene
                        )
        {
          item_by_name_type places (name_map_for_items (scene->all_places()));
          weaver::transition wt
            (transition.document(), scene, places, transition.id());
          from::transition (&wt, transition.id());
        }

        void place ( const data::handle::place& place
                   , ui::graph::scene_type* scene
                   )
        {
          //! \note Does not need actual list, as it only adds itself.
          item_by_name_type place_by_name;

          ui::graph::place_item* item (new ui::graph::place_item (place));
          scene->addItem (item);
          weaver::place wp (item, place_by_name);
          from::place (&wp, place.id());
        }

        void top_level_port ( const data::handle::port& port
                            , ui::graph::scene_type* scene
                            )
        {
          item_by_name_type places (name_map_for_items (scene->all_places()));
          weaver::port_toplevel wptl (scene, places, port.document());
          from::port (&wptl, port.id());
        }
      }
    }
  }
}
