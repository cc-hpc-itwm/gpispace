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

        class transition
        {
        public:
          explicit transition ( data::internal_type* root
                              , ui::graph::scene_type* scene
                              )
            : _scene (scene)
            , _root (root)
          { }

          template<int Type, typename T> void weave (const T & x) {}
          template<int Type> void weave () {}

        private:
          ui::graph::scene_type* _scene;
          ui::graph::transition_item* _transition;
          data::internal_type* _root;
        };

        WSIG (transition, transition::open, xml::parse::id::ref::transition, id)
        {
          const ::xml::parse::type::transition_type& trans (id.get());

          _transition = new ui::graph::transition_item
            (data::handle::transition (id, _root));

          initialize_and_set_position (_transition, id);
          _scene->addItem (_transition);

          from::many ( this
                     , trans.resolved_function().get().ports().ids()
                     , from::port
                     );

          from::many (this, trans.connections().ids(), from::connection);

          //! \todo do something if not already set
          //        _transition->repositionChildrenAndResize();

        }

        WSIG (transition, connection::open, ::xml::parse::id::ref::connect, id)
        {
          _scene->create_connection (data::handle::connect (id, _root));
        }

        WSIG (transition, port::open, ::xml::parse::id::ref::port, id)
        {
          const ::xml::parse::type::port_type& port (id.get());

          ui::graph::port_item* item
            ( new ui::graph::port_item
              (data::handle::port (id, _root), _transition)
            );

          //! \todo This sets the wrong position: differentiate
          //! between ports on transition and ports in net (inner, outer)
          initialize_and_set_position (item, id);
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

          template<int Type, typename T> void weave (const T & x) {}
          template<int Type> void weave () {}

        private:
          ui::graph::scene_type* _scene;
          data::internal_type* _root;
        };

        WSIG (port_toplevel, port::open, ::xml::parse::id::ref::port, id)
        {
          ui::graph::port_item* port_item
            (new ui::graph::top_level_port_item (data::handle::port (id, _root)));

          initialize_and_set_position (port_item, id);
          _scene->addItem (port_item);

          _scene->create_port_place_association (data::handle::port (id, _root));
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

          template<int Type, typename T> void weave (const T & x) {}
          template<int Type> void weave () {}

        private:
          ui::graph::scene_type* _scene;

          ::xml::parse::id::ref::function _function;

          data::internal_type* _root;
        };

        WSIG (net, net::open, ::xml::parse::id::ref::net, id)
        {
          const ::xml::parse::type::net_type& net (id.get());
          from::many (this, net.places().ids(), from::place);
          from::many (this, net.transitions().ids(), from::transition);
          from::many (this, _function.get().ports().ids(), from::port);
        }

        WSIG (net, transition::open, ::xml::parse::id::ref::transition, id)
        {
          display::transition (data::handle::transition (id, _root), _scene);
        }

        WSIG (net, place::open, ::xml::parse::id::ref::place, id)
        {
          display::place (data::handle::place (id, _root), _scene);
        }

        WSIG (net, port::open, ::xml::parse::id::ref::port, id)
        {
          display::top_level_port (data::handle::port (id, _root), _scene);
        }
      }

      namespace display
      {
        ui::graph::scene_type* net ( const data::handle::net& net
                                   , const data::handle::function& parent
                                   )
        {
          ui::graph::scene_type* scene (new ui::graph::scene_type (net, parent));

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
          initialize_and_set_position (item, place.id());
          scene->addItem (item);
        }

        void top_level_port ( const data::handle::port& port
                            , ui::graph::scene_type* scene
                            )
        {
          weaver::port_toplevel wptl (scene, port.document());
          from::port (&wptl, port.id());
        }
      }
    }
  }
}
