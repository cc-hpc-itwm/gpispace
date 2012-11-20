// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_DATA_PROXY_HPP
#define _PNETE_DATA_PROXY_HPP 1

#include <boost/variant.hpp>

#include <xml/parse/type/function.hpp>
#include <xml/parse/type/expression.hpp>
#include <xml/parse/type/mod.hpp>
#include <xml/parse/type/net.hpp>
#include <xml/parse/type/port.hpp>
#include <xml/parse/type_map_type.hpp>

#include <QString>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class document_view;

      namespace graph
      {
        class scene_type;
      }
    }

    namespace data
    {
      class internal_type;

      namespace proxy
      {
        template<typename DATA, typename DISPLAY = void>
        class proxy_base
        {
        public:
          typedef DATA data_type;
          typedef DISPLAY display_type;

          proxy_base ( internal_type* root
                     , data_type data
                     , const ::xml::parse::id::ref::function& function
                     , display_type* display = NULL
                     )
            : _data (data)
            , _function (function)
            , _display (display)
            , _root (root)
          {}
          data_type& data()
          {
            return _data;
          }
          ::xml::parse::type::function_type& function()
          {
            return _function.get_ref();
          }

          display_type* display()
          {
            return _display;
          }
          internal_type* root() const
          {
            return _root;
          }

        private:
          data_type _data;
          ::xml::parse::id::ref::function _function;
          display_type* _display;

          internal_type* _root;
        };

        namespace data
        {
          namespace xml_type = ::xml::parse::type;

          class expression_type
          {
          private:
            xml_type::expression_type& _expression;
            xml_type::function_type::ports_type& _in;
            xml_type::function_type::ports_type& _out;

          public:
            explicit expression_type
              ( xml_type::expression_type& expression
              , xml_type::function_type::ports_type& in
              , xml_type::function_type::ports_type& out
              );

            xml_type::expression_type& expression ();
            xml_type::function_type::ports_type& in ();
            xml_type::function_type::ports_type& out ();
          };

          class module_type
          {
          private:
            xml_type::module_type& _mod;
            xml_type::function_type::ports_type& _in;
            xml_type::function_type::ports_type& _out;

          public:
            explicit module_type
              ( xml_type::module_type& mod
              , xml_type::function_type::ports_type& in
              , xml_type::function_type::ports_type& out
              );

            xml_type::module_type& mod ();
            xml_type::function_type::ports_type& in ();
            xml_type::function_type::ports_type& out ();
          };
        }

        typedef proxy_base<data::expression_type> expression_proxy;
        typedef proxy_base<data::module_type> mod_proxy;
        typedef proxy_base< ::xml::parse::id::ref::net
                          , ui::graph::scene_type
                          > net_proxy;

        typedef boost::variant<expression_proxy, mod_proxy, net_proxy> type;

        ::xml::parse::type::function_type& function (type&);
        ::fhg::pnete::data::internal_type* root (const type&);

        ui::document_view* document_view_factory (type&);
      }
    }
  }
}

#endif
