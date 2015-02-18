// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/weaver/display.hpp>

#include <pnete/data/handle/connect.hpp>
#include <pnete/data/handle/function.hpp>
#include <pnete/data/handle/net.hpp>
#include <pnete/data/handle/place.hpp>
#include <pnete/data/handle/place_map.hpp>
#include <pnete/data/handle/port.hpp>
#include <pnete/ui/graph/base_item.fwd.hpp>
#include <pnete/ui/graph/connectable_item.hpp>
#include <pnete/ui/graph/place.hpp>
#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/port_place_association.hpp>
#include <pnete/ui/graph/scene.hpp>
#include <pnete/ui/graph/transition.hpp>
#include <pnete/weaver/weaver.hpp>

#include <fhg/util/num.hpp>
#include <fhg/util/parse/require.hpp>

#include <util/qt/cast.hpp>

#include <we/type/property.hpp>

#include <QtGlobal>

#include <unordered_map>

namespace fhg
{
  namespace pnete
  {
    namespace weaver
    {
      namespace
      {
        template<typename ID>
          bool is_hard_hidden (const ID& id)
        {
          return boost::get<bool> ( id.get().properties().get
                                      ({"fhg", "pnete", "is_hard_hidden"})
                                  . get_value_or (false)
                                  );
        }

        template<typename ID_TYPE>
          void initialize_and_set_position ( ui::graph::base_item* item
                                           , const ID_TYPE& id
                                           , const bool outer = false
                                           )
        {
          const std::string var_name (!outer ? "position" : "outer_position");

          if (!id.get().properties().get ({"fhg", "pnete", var_name, "x"}))
          {
            id.get_ref().properties().set ({"fhg", "pnete", var_name, "x"}, 0.0);
          }
          if (!id.get().properties().get ({"fhg", "pnete", var_name, "y"}))
          {
            id.get_ref().properties().set ({"fhg", "pnete", var_name, "y"}, 0.0);
          }

          item->set_just_pos_but_not_in_property
            ( boost::get<double>
              (*id.get().properties().get ({"fhg", "pnete", var_name, "x"}))
            , boost::get<double>
              (*id.get().properties().get ({"fhg", "pnete", var_name, "y"}))
            );
        }

        class transition
        {
        public:
          explicit transition ( data::internal_type* root
                              , ui::graph::scene_type* scene
                              )
            : _scene (scene)
            , _root (root)
          { }

          template<int Type, typename T> void weave (const T &) {}
          template<int Type> void weave () {}

        private:
          ui::graph::scene_type* _scene;
          ui::graph::transition_item* _transition;
          data::internal_type* _root;
        };

        WSIG (transition, transition::open, xml::parse::id::ref::transition, id)
        {
          if (is_hard_hidden (id))
          {
            return;
          }

          const ::xml::parse::type::transition_type& trans (id.get());

          _transition = new ui::graph::transition_item
            (data::handle::transition (id, _root));

          _scene->addItem (_transition);
          initialize_and_set_position (_transition, id);

          from::many_port (this, trans.resolved_function().get().ports().ids());
          from::many_connection (this, trans.connections().ids());
          from::many_place_map (this, trans.place_map().ids());

          //! \todo do something if not already set
          //        _transition->repositionChildrenAndResize();

        }

        WSIG (transition, connection::open, ::xml::parse::id::ref::connect, id)
        {
          if (is_hard_hidden (id))
          {
            return;
          }

          _scene->create_connection (data::handle::connect (id, _root));
        }

        WSIG (transition, place_map::open, ::xml::parse::id::ref::place_map, id)
        {
          if (is_hard_hidden (id))
          {
            return;
          }

          _scene->create_place_map (data::handle::place_map (id, _root));
        }

        WSIG (transition, port::open, ::xml::parse::id::ref::port, id)
        {
          if (is_hard_hidden (id))
          {
            return;
          }

          display::port (data::handle::port (id, _root), _transition);
        }

        class port_toplevel
        {
        public:
          explicit port_toplevel ( ui::graph::scene_type* scene
                                 , data::internal_type* root
                                 )
            : _scene (scene)
            , _root (root)
          { }

          template<int Type, typename T> void weave (const T &) {}
          template<int Type> void weave () {}

        private:
          ui::graph::scene_type* _scene;
          data::internal_type* _root;
        };

        WSIG (port_toplevel, port::open, ::xml::parse::id::ref::port, id)
        {
          if (is_hard_hidden (id))
          {
            return;
          }

          data::handle::port port (id, _root);

          if (port.is_tunnel())
          {
            return;
          }

          ui::graph::base_item* item (new ui::graph::top_level_port_item (port));

          _scene->addItem (item);
          initialize_and_set_position (item, id);

          _scene->create_port_place_association (port);
        }

        class net
        {
        public:
          explicit net ( data::internal_type* root
                       , ui::graph::scene_type* scene
                       , const ::xml::parse::id::ref::function& function
                       )
            : _scene (scene)
            , _function (function)
            , _root (root)
          { }

          template<int Type, typename T> void weave (const T &) {}
          template<int Type> void weave () {}

        private:
          ui::graph::scene_type* _scene;

          ::xml::parse::id::ref::function _function;

          data::internal_type* _root;
        };

        WSIG (net, net::open, ::xml::parse::id::ref::net, id)
        {
          if (is_hard_hidden (id))
          {
            return;
          }

          const ::xml::parse::type::net_type& net (id.get());
          from::many_place (this, net.places().ids());
          from::many_transition (this, net.transitions().ids());
          from::many_port (this, _function.get().ports().ids());
        }

        WSIG (net, transition::open, ::xml::parse::id::ref::transition, id)
        {
          if (is_hard_hidden (id))
          {
            return;
          }

          display::transition (data::handle::transition (id, _root), _scene);
        }

        WSIG (net, place::open, ::xml::parse::id::ref::place, id)
        {
          if (is_hard_hidden (id))
          {
            return;
          }

          display::place (data::handle::place (id, _root), _scene);
        }

        WSIG (net, port::open, ::xml::parse::id::ref::port, id)
        {
          if (is_hard_hidden (id))
          {
            return;
          }

          display::top_level_port (data::handle::port (id, _root), _scene);
        }
      }

      namespace display
      {
        ui::graph::scene_type* net ( data::manager& data_manager
                                   , const data::handle::net& net
                                   , const data::handle::function& parent
                                   )
        {
          ui::graph::scene_type* scene
            (new ui::graph::scene_type (data_manager, net, parent));

          weaver::net wn (net.document(), scene, parent.id());
          from::net (&wn, net.id());

          return scene;
        }

        void transition ( const data::handle::transition& transition
                        , ui::graph::scene_type* scene
                        )
        {
          weaver::transition wt (transition.document(), scene);
          from::transition (&wt, transition.id());
        }

        void place ( const data::handle::place& place
                   , ui::graph::scene_type* scene
                   )
        {
          ui::graph::place_item* item (new ui::graph::place_item (place));
          scene->addItem (item);
          initialize_and_set_position (item, place.id());
        }

        void top_level_port ( const data::handle::port& port
                            , ui::graph::scene_type* scene
                            )
        {
          weaver::port_toplevel wptl (scene, port.document());
          from::port (&wptl, port.id());
        }

        void port (const data::handle::port& port, ui::graph::transition_item* i)
        {
          ui::graph::port_item* item (new ui::graph::port_item (port, i));

          initialize_and_set_position (item, port, true);
        }

        void place_map ( const data::handle::place_map& place_map
                       , ui::graph::scene_type* scene
                       )
        {
          weaver::transition wt (place_map.document(), scene);
          from::place_map (&wt, place_map.id());
        }

        void connection ( const data::handle::connect& connection
                        , ui::graph::scene_type* scene
                        )
        {
          weaver::transition wt (connection.document(), scene);
          from::connection (&wt, connection.id());
        }
      }
    }
  }
}
