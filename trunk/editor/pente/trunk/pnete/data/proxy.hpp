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
      class document_view;
      namespace graph
      {
        class scene;
      }
    }

    namespace data
    {
      class internal_type;

      namespace proxy
      {
        typedef ::xml::parse::type::function_with_mapping_type
                function_with_mapping_type;

        template<typename DATA, typename DISPLAY = void>
        class proxy_base
        {
        public:
          typedef DATA data_type;
          typedef DISPLAY display_type;

          proxy_base ( internal_type* root
                     , data_type data
                     , function_with_mapping_type& function_with_mapping
                     , display_type* display = NULL
                     )
            : _data (data)
            , _function_with_mapping (function_with_mapping)
            , _display (display)
            , _root (root)
          {}
          data_type& data()
          {
            return _data;
          }
          ::xml::parse::type::function_type& function()
          {
            return _function_with_mapping.function();
          }
          ::xml::parse::type::type_map_type& type_map()
          {
            return _function_with_mapping.type_map();
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
          function_with_mapping_type _function_with_mapping;
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
            explicit net_type (xml_type::net_type& net);

            xml_type::net_type& net ();
          };
        }

        typedef proxy_base<data::expression_type> expression_proxy;
        typedef proxy_base<data::mod_type> mod_proxy;
        typedef proxy_base<data::net_type, ui::graph::scene> net_proxy;

        typedef boost::variant<expression_proxy, mod_proxy, net_proxy> type;

        ::xml::parse::type::function_type& function (type&);
        ::fhg::pnete::data::internal_type* root (const type&);

        ui::document_view* document_view_factory (type&);

        namespace visitor
        {
          typedef ::fhg::pnete::data::internal_type* internal_type_ptr_type;

          class root
            : public boost::static_visitor<internal_type_ptr_type>
          {
          public:
            template<typename T>
            internal_type_ptr_type operator () (const T& x) const
              {
                return x.root();
              }
          };

          template<typename RES>
          class function : public boost::static_visitor<RES>
          {
          public:
            template<typename T> RES operator () (T& x) const
            {
              return x.function();
            }
          };

          template<typename RES>
          class type_map : public boost::static_visitor<RES>
          {
          public:
            template<typename T> RES operator () (T& x) const
            {
              return x.type_map();
            }
          };

          class document_view_factory
            : public boost::static_visitor<ui::document_view*>
          {
          private:
            type& _proxy;

          public:
            document_view_factory (type& proxy);
            ui::document_view * operator () (expression_proxy &) const;
            ui::document_view * operator () (mod_proxy &) const;
            ui::document_view * operator () (net_proxy &) const;
          };
        }
      }
    }
  }
}

#endif
