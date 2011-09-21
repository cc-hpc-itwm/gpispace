// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_DATA_PROXY_HPP
#define _PNETE_DATA_PROXY_HPP 1

#include <boost/variant.hpp>

#include <xml/parse/types.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      class Scene;
    }
    namespace ui
    {
      class document_widget;
    }

    namespace data
    {
      namespace proxy
      {
        typedef ::xml::parse::type::function_type function_type;

        template<typename DATA, typename DISPLAY = void>
        class proxy_base
        {
        public:
          typedef DATA data_type;
          typedef DISPLAY display_type;

          proxy_base ( data_type data
                     , function_type& function
                     , display_type* display = NULL
                     )
            : _data (data)
            , _function (function)
            , _display (display)
          {}
          data_type& data() { return _data; }
          function_type& function() { return _function; }
          const function_type& function() const { return _function; }
          display_type* display() { return _display; }

        private:
          data_type _data;
          function_type& _function;
          display_type* _display;
        };

        namespace xml_type = ::xml::parse::type;

        namespace data
        {
          class expression_type
          {
          private:
            xml_type::expression_type& _expression;
            xml_type::ports_type& _in;
            xml_type::ports_type& _out;

          public:
            explicit expression_type ( xml_type::expression_type& expression
                                     , xml_type::ports_type& in
                                     , xml_type::ports_type& out
                                     );

            xml_type::expression_type& expression ();
            xml_type::ports_type& in ();
            xml_type::ports_type& out ();
          };

          class mod_type
          {
          private:
            xml_type::mod_type& _mod;
            xml_type::ports_type& _in;
            xml_type::ports_type& _out;

          public:
            explicit mod_type ( xml_type::mod_type& mod
                              , xml_type::ports_type& in
                              , xml_type::ports_type& out
                              );

            xml_type::mod_type& mod ();
            xml_type::ports_type& in ();
            xml_type::ports_type& out ();
          };

          class net_type
          {
          private:
            xml_type:: net_type& _net;

          public:
            explicit net_type ( xml_type::net_type& net
                              );

            xml_type::net_type& net ();
          };
        }

        typedef proxy_base<data::expression_type> expression_proxy;
        typedef proxy_base<data::mod_type> mod_proxy;
        typedef proxy_base<data::net_type, graph::Scene> net_proxy;

        typedef boost::variant<expression_proxy, mod_proxy, net_proxy> type;

        const function_type& function (const type &);

        namespace visitor
        {
          class function : public boost::static_visitor<const function_type&>
          {
          public:
            template<typename T>
            const function_type& operator () (const T & x) const
            {
              return x.function();
            }
          };

          class document_widget_factory
            : public boost::static_visitor<ui::document_widget *>
          {
          private:
            type& _proxy;
          public:
            document_widget_factory (type& proxy);
            ui::document_widget * operator () (expression_proxy &) const;
            ui::document_widget * operator () (mod_proxy &) const;
            ui::document_widget * operator () (net_proxy &) const;
          };
        }
      }
    }
  }
}

#endif
