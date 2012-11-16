// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _FHG_PNETE_WEAVER_DISPLAY_HPP
#define _FHG_PNETE_WEAVER_DISPLAY_HPP 1

#include <pnete/data/proxy.hpp>

#include <pnete/weaver/weaver.hpp>

#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/connectable_item.hpp>

#include <boost/unordered_map.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      class internal_type;
    }

    namespace ui
    {
      namespace graph
      {
        class place_item;
        class transition_item;
        class scene_type;
      }
    }
    namespace weaver
    {
      typedef boost::unordered_map< std::string
                                  , ui::graph::connectable_item*
                                  > item_by_name_type;

      typedef XMLTYPE(function_with_mapping_type) function_with_mapping_type;

      class function
      {
      public:
        explicit function ( function_with_mapping_type
                          , data::internal_type*
                          );

        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

        data::proxy::type* proxy () const;

        XMLTYPE(function_type::ports_type)& in ();
        XMLTYPE(function_type::ports_type)& out ();

      private:
        data::proxy::type* _proxy;
        function_with_mapping_type _function_with_mapping;

        struct ports_type
        {
          XMLTYPE(function_type::ports_type)* in;
          XMLTYPE(function_type::ports_type)* out;

          ports_type() : in (NULL), out (NULL) {}
        } _ports;

        ui::graph::scene_type* _scene;
        data::internal_type* _root;
      };

      class net
      {
      public:
        explicit net ( data::internal_type*
                     , ui::graph::scene_type* scene
                     , const ::xml::parse::id::ref::net& net
                     , XMLTYPE(function_type::ports_type)& in
                     , XMLTYPE(function_type::ports_type)& out
                     , const ::xml::parse::id::ref::function& function
                     );

        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

      private:
        ui::graph::scene_type* _scene;

        ::xml::parse::id::ref::net _net;
        XMLTYPE(function_type::ports_type)& _in;
        XMLTYPE(function_type::ports_type)& _out;
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

        boost::optional< ::xml::parse::type::type_map_type&> _type_map;
        boost::optional< ::xml::parse::type::function_type&> _function;

        function_with_mapping_type
        get_function (XMLTYPE(transition_type::function_or_use_type)& f);
      };

      class property
      {
      public:
        explicit property (ui::graph::base_item*);
        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

      private:
        ui::graph::base_item* _item;
        WETYPE(property::path_type) _path;
      };

      class connection
      {
      public:
        explicit connection ( ui::graph::scene_type*
                            , item_by_name_type& place_item_by_name
                            , item_by_name_type& port_item_by_name
                            , const ui::graph::connectable::direction::type& direction
                            , const bool& read = false
                            );

        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

      private:
        ui::graph::scene_type* _scene;
        item_by_name_type& _place_item_by_name;
        item_by_name_type& _port_item_by_name;
        const ui::graph::connectable::direction::type _direction;
        const bool _read;
        std::string _port;
        std::string _place;
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
                               , const ::xml::parse::id::ref::function&
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
        ::xml::parse::id::ref::function _function;
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
