// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _FHG_PNETE_WEAVER_DISPLAY_HPP
#define _FHG_PNETE_WEAVER_DISPLAY_HPP 1

#include <xml/parse/types.hpp>

#include <pnete/data/proxy.hpp>

#include <pnete/weaver/weaver.hpp>

#include <pnete/ui/graph/port.hpp>

#include <boost/optional.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class place;
        class scene;
        class transition;
        class connection;
      }
    }
    namespace weaver
    {
      class function
      {
      public:
        explicit function (data::proxy::function_type &);

        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

        data::proxy::type* proxy () const;

        XMLTYPE(ports_type)& in ();
        XMLTYPE(ports_type)& out ();

      private:
        data::proxy::type* _proxy;
        data::proxy::function_type& _fun;

        struct
        {
          XMLTYPE(ports_type) in;
          XMLTYPE(ports_type) out;
        } _ports;

        ui::graph::scene* _scene;
      };

      class net
      {
      public:
        explicit net ( ui::graph::scene* scene
                     , XMLTYPE(net_type)& net
                     , XMLTYPE(ports_type)& in
                     , XMLTYPE(ports_type)& out
                     );

        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

        boost::optional< XMLTYPE(function_type)&>
        get_function (const std::string&);

      private:
        ui::graph::scene* _scene;

        XMLTYPE(net_type)& _net;
      };

      class expression
      {
      public:
        explicit expression ( XMLTYPE(expressions_type)& exp
                            , XMLTYPE(ports_type)& in
                            , XMLTYPE(ports_type)& out
                            );

        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}
      };

      class transition
      {
      public:
        explicit transition ( ui::graph::transition*
                            , XMLTYPE(net_type)&
                            );

        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

        XMLTYPE(function_type)&
        get_function (const XMLTYPE(transition_type::f_type) &);

      private:
        ui::graph::transition* _transition;
        ui::graph::port::DIRECTION _current_port_direction;
        XMLTYPE(net_type)& _net;
      };

      class port
      {
      public:
        explicit port (ui::graph::port*);

        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

      private:
        ui::graph::port* _port;
      };

      class port_toplevel
      {
      public:
        explicit port_toplevel ( ui::graph::scene* const
                               , const ui::graph::port::DIRECTION&
                               );

        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

      private:
        ui::graph::scene* const _scene;
        const ui::graph::port::DIRECTION& _current_direction;
      };

      class place
      {
      public:
        explicit place (ui::graph::place*);

        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

      private:
        ui::graph::place* _place;
      };

      namespace visitor
      {
        typedef XMLTYPE(function_type) function_type;

        function_type& default_fun();

        class get_function : public boost::static_visitor<function_type&>
        {
        private:
          XMLTYPE(net_type)& _net;

        public:
          get_function (XMLTYPE(net_type)& net) : _net (net) {}

          function_type& operator () (const function_type& fun) const
          {
            return const_cast<function_type &> (fun);
          }

          function_type& operator () (const XMLTYPE(use_type)& use) const
          {
            boost::optional<function_type&> f (_net.get_function (use.name));

            if (f)
              {
                return *f;
              }
            else
              {
                //! \todo do something more clever, generate error message!?

                return default_fun();
              };
          }
        };
      }
    }
  }
}

#include <pnete/weaver/display.cpp>

#endif
