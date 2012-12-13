// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef FHG_PNETE_WEAVER_DISPLAY_HPP
#define FHG_PNETE_WEAVER_DISPLAY_HPP

#include <pnete/data/proxy.fwd.hpp>
#include <pnete/data/internal.fwd.hpp>

#include <pnete/ui/graph/base_item.fwd.hpp>
#include <pnete/ui/graph/connectable_item.hpp>
#include <pnete/ui/graph/place.fwd.hpp>
#include <pnete/ui/graph/port.fwd.hpp>
#include <pnete/ui/graph/scene.fwd.hpp>
#include <pnete/ui/graph/transition.fwd.hpp>

#include <we/type/property.fwd.hpp>

#include <xml/parse/id/types.hpp>

#include <boost/unordered_map.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace weaver
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
                            , ui::graph::transition_item*
                            , const ::xml::parse::id::ref::net&
                            , item_by_name_type&
                            );

        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

      private:
        ui::graph::scene_type* _scene;
        ui::graph::transition_item* _transition;
        ui::graph::connectable::direction::type _current_port_direction;
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
                               , const ui::graph::connectable::direction::type&
                               , item_by_name_type& place_item_by_name
                               , data::internal_type* root
                               );

        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

      private:
        ui::graph::scene_type* _scene;
        item_by_name_type& _place_item_by_name;
        std::string _name;
        const ui::graph::connectable::direction::type _direction;
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
    }
  }
}

#endif
