// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/data/proxy.hpp>

#include <QObject>

#include <boost/variant.hpp>

#include <pnete/ui/document_view.hpp>
#include <pnete/ui/expression_view.hpp>
#include <pnete/ui/mod_view.hpp>
#include <pnete/ui/net_view.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace proxy
      {
        namespace visitor
        {
          class root
            : public boost::static_visitor<internal_type*>
          {
          public:
            template<typename T>
            internal_type* operator () (const T& x) const
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

        ::xml::parse::type::function_type& function (type& proxy)
        {
          return boost::apply_visitor
            ( visitor::function< ::xml::parse::type::function_type&> ()
            , proxy
            );
        }

        ::fhg::pnete::data::internal_type* root (const type& proxy)
        {
          return boost::apply_visitor (visitor::root (), proxy);
        }

        namespace data
        {
          expression_type::expression_type
            ( xml_type::expression_type& expression
            , xml_type::ports_type& in
            , xml_type::ports_type& out
            )
              : _expression (expression)
              , _in (in)
              , _out (out)
          {}
          xml_type::expression_type& expression_type::expression ()
          {
            return _expression;
          }
          xml_type::ports_type& expression_type::in () { return _in; }
          xml_type::ports_type& expression_type::out () { return _out; }

          module_type::module_type ( xml_type::module_type& mod
                             , xml_type::ports_type& in
                             , xml_type::ports_type& out
                             )
            : _mod (mod)
            , _in (in)
            , _out (out)
          {}
          xml_type::module_type& module_type::mod () { return _mod; }
          xml_type::ports_type& module_type::in () { return _in; }
          xml_type::ports_type& module_type::out () { return _out; }

          net_type::net_type ( xml_type::net_type& net
                             )
            : _net (net)
          {}
          xml_type::net_type& net_type::net () { return _net; }
        }

        ui::document_view* document_view_factory (type& proxy)
        {
          return boost::apply_visitor
            (visitor::document_view_factory (proxy), proxy);
        }

        namespace visitor
        {
          document_view_factory::document_view_factory (type & proxy)
            : _proxy (proxy)
          {}

          ui::document_view*
          document_view_factory::operator () (expression_proxy & proxy) const
          {
            return new ui::expression_view ( _proxy
                                           , proxy.data()
                                           );
          }

          ui::document_view*
          document_view_factory::operator () (mod_proxy & proxy) const
          {
            return new ui::mod_view ( _proxy
                                    , proxy.data()
                                    );
          }

          ui::document_view*
          document_view_factory::operator () (net_proxy & proxy) const
          {
            return new ui::net_view ( _proxy
                                    , proxy.data()
                                    , proxy.display()
                                    );
          }
        }
      }
    }
  }
}
