// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _FHG_PNETE_WEAVER_DISPLAY_HPP
#define _FHG_PNETE_WEAVER_DISPLAY_HPP 1

#include <xml/parse/types.hpp>

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
        namespace place { class item; }
        class scene;
        namespace transition { class item; }
      }
    }
    namespace weaver
    {
      typedef boost::unordered_map< std::string
                                  , ui::graph::connectable::item*
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

        XMLTYPE(ports_type)& in ();
        XMLTYPE(ports_type)& out ();

      private:
        data::proxy::type* _proxy;
        function_with_mapping_type _function_with_mapping;

        struct ports_type
        {
          XMLTYPE(ports_type)* in;
          XMLTYPE(ports_type)* out;

          ports_type() : in (NULL), out (NULL) {}
        } _ports;

        ui::graph::scene* _scene;
        data::internal_type* _root;
      };

      WSIG(function, expression::open, XMLTYPE(expression_type), exp);
      WSIG(function, mod::open, XMLTYPE(mod_type), mod);
      WSIG(function, net::open, XMLTYPE(net_type), net);
      WSIG(function, function::in, XMLTYPE(ports_type), in);
      WSIG(function, function::out, XMLTYPE(ports_type), out);
      WSIG(function, function::fun, XMLTYPE(function_type::type), fun);

      class net
      {
      public:
        explicit net ( data::internal_type*
                     , ui::graph::scene* scene
                     , XMLTYPE(net_type)& net
                     , XMLTYPE(ports_type)& in
                     , XMLTYPE(ports_type)& out
                     );

        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

      private:
        ui::graph::scene* _scene;

        XMLTYPE(net_type)& _net;
        XMLTYPE(ports_type)& _in;
        XMLTYPE(ports_type)& _out;

        item_by_name_type _place_item_by_name;
        data::internal_type* _root;
      };

      WSIG( net
          , net::transitions
          , XMLTYPE(net_type::transitions_type)
          , transitions
          );
      WSIG(net, net::places, XMLTYPE(net_type::places_type), places);
      WSIG(net, place::open, ITVAL(XMLTYPE(net_type::places_type)), place);
      WSIG( net
          , transition::open
          , ITVAL(XMLTYPE(net_type::transitions_type))
          , transition
          );
      WSIGE(net, net::close);

      class transition
      {
      public:
        explicit transition ( data::internal_type*
                            , ui::graph::scene*
                            , ui::graph::transition::item*
                            , XMLTYPE(net_type)&
                            , item_by_name_type&
                            );

        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

      private:
        ui::graph::scene* _scene;
        ui::graph::transition::item* _transition;
        ui::graph::connectable::direction::type _current_port_direction;
        XMLTYPE(net_type)& _net;

        item_by_name_type& _place_item_by_name;
        item_by_name_type _port_in_item_by_name;
        item_by_name_type _port_out_item_by_name;
        data::internal_type* _root;

        boost::optional< ::xml::parse::type::type_map_type&> _type_map;

        function_with_mapping_type
        get_function (XMLTYPE(transition_type::f_type)& f);
      };

      WSIG(transition, transition::name, std::string, name);
      WSIG( transition
          , transition::function
          , XMLTYPE(transition_type::f_type)
          , fun
          );
      WSIG(transition, port::open, ITVAL(XMLTYPE(ports_type)), port);
      WSIG(transition, transition::connect_read, XMLTYPE(connections_type), cs);
      WSIG(transition, transition::connect_in, XMLTYPE(connections_type), cs);
      WSIG(transition, transition::connect_out, XMLTYPE(connections_type), cs);
      WSIG(transition, transition::properties, WETYPE(property::type), prop);

      class property
      {
      public:
        explicit property (ui::graph::item*);
        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

      private:
        ui::graph::item* _item;
        WETYPE(property::path_type) _path;
      };

      WSIG(property, properties::open, WETYPE(property::type), prop);
      WSIG(property, property::open, WETYPE(property::key_type), key);
      WSIG(property, property::value, WETYPE(property::value_type), value);
      WSIGE(property, property::close);

      class connection
      {
      public:
        explicit connection ( ui::graph::scene*
                            , item_by_name_type& place_item_by_name
                            , item_by_name_type& port_item_by_name
                            , const ui::graph::connectable::direction::type& direction
                            , const bool& read = false
                            );

        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

      private:
        ui::graph::scene* _scene;
        item_by_name_type& _place_item_by_name;
        item_by_name_type& _port_item_by_name;
        const ui::graph::connectable::direction::type _direction;
        const bool _read;
        std::string _port;
        std::string _place;
      };

      WSIG(connection, connection::port, std::string, port);
      WSIG(connection, connection::place, std::string, place);
      WSIGE(connection, connection::close);

      class port
      {
      public:
        explicit port (ui::graph::port::item*, item_by_name_type&);

        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

      private:
        ui::graph::port::item* _port;

        item_by_name_type& _port_item_by_name;
      };

      WSIG(port, port::name, std::string, name);
      WSIG(port, port::type, std::string, type);
      WSIG(port, port::properties, WETYPE(property::type), props);

      class port_toplevel
      {
      public:
        explicit port_toplevel ( ui::graph::scene*
                               , const ui::graph::connectable::direction::type&
                               , item_by_name_type& place_item_by_name
                               );

        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

      private:
        ui::graph::scene* _scene;
        item_by_name_type& _place_item_by_name;
        std::string _name;
        const ui::graph::connectable::direction::type _direction;
        ui::graph::port::item* _port_item;
      };

      WSIG(port_toplevel, port::open, ITVAL(XMLTYPE(ports_type)), port);
      WSIG(port_toplevel, port::name, std::string, name);
      WSIG(port_toplevel, port::type, std::string, type);
      WSIG(port_toplevel, port::place, MAYBE(std::string), place);
      WSIG(port_toplevel, port::properties, WETYPE(property::type), props);

      class place
      {
      public:
        explicit place (ui::graph::place::item*, item_by_name_type&);

        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

      private:
        ui::graph::place::item* _place;

        item_by_name_type& _place_item_by_name;
      };

      WSIG(place, place::name, std::string, name);
      WSIG(place, place::type, std::string, type);
      WSIG(place, place::properties, WETYPE(property::type), props);

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
          operator() (const XMLTYPE(function_type)& fun) const
          {
            return function_with_mapping_type
              (const_cast< ::xml::parse::type::function_type &> (fun));
          }

          function_with_mapping_type
          operator() (const XMLTYPE(use_type)& use) const
          {
            return _net.get_function (use.name);
          }
        };
      }
    }
  }
}

#endif
