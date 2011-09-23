// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_DATA_PROXY_HPP
#define _PNETE_DATA_PROXY_HPP 1

#include <boost/variant.hpp>

#include <xml/parse/types.hpp>

#include <QString>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class document_widget;
      namespace graph
      {
        class scene;
      }
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

          QString name () const
          {
            return _function.name
              ? QString ((*_function.name).c_str())
              : QString ()
              ;
          }

          const QString& name (const QString& name_)
          {
            if (!name_.isEmpty())
            {
              _function.name = name_.toStdString();
            }
            else
            {
              _function.name.clear();
            }

            return name_;
          }

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
        typedef proxy_base<data::net_type, ui::graph::scene> net_proxy;

        typedef boost::variant<expression_proxy, mod_proxy, net_proxy> type;

        const function_type& function (const type &);
        QString name (const type&);
        const QString& name (type&, const QString&);

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

          class name : public boost::static_visitor<QString>
          {
          public:
            template<typename T>
            QString operator () (const T & x) const { return x.name(); }
          };

          class set_name : public boost::static_visitor<const QString&>
          {
          private:
            const QString& _name;

          public:
            set_name (const QString& name_) : _name (name_) {}

            template<typename T>
            const QString& operator () (T& x) const
            {
              return x.name(_name);
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
