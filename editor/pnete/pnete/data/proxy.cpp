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
        namespace
        {
          class visitor_root : public boost::static_visitor<internal_type*>
          {
          public:
            template<typename T>
            internal_type* operator () (const T& x) const
            {
              return x.root();
            }
          };

          class visitor_function
            : public boost::static_visitor<const ::xml::parse::id::ref::function&>
          {
          public:
            template<typename T>
              const ::xml::parse::id::ref::function& operator() (T& x) const
            {
              return x.function();
            }
          };
        }

        ::xml::parse::type::function_type& function (type& proxy)
        {
          return boost::apply_visitor (visitor_function(), proxy).get_ref();
        }

        ::fhg::pnete::data::internal_type* root (const type& proxy)
        {
          return boost::apply_visitor (visitor_root(), proxy);
        }

        namespace data
        {
          expression_type::expression_type
            ( xml_type::expression_type& expression
            , xml_type::function_type::ports_type& in
            , xml_type::function_type::ports_type& out
            )
              : _expression (expression)
              , _in (in)
              , _out (out)
          {}
          xml_type::expression_type& expression_type::expression ()
          {
            return _expression;
          }
          xml_type::function_type::ports_type& expression_type::in () { return _in; }
          xml_type::function_type::ports_type& expression_type::out () { return _out; }

          module_type::module_type ( xml_type::module_type& mod
                             , xml_type::function_type::ports_type& in
                             , xml_type::function_type::ports_type& out
                             )
            : _mod (mod)
            , _in (in)
            , _out (out)
          {}
          xml_type::module_type& module_type::mod () { return _mod; }
          xml_type::function_type::ports_type& module_type::in () { return _in; }
          xml_type::function_type::ports_type& module_type::out () { return _out; }
        }

        namespace
        {
          class document_view_for_proxy
            : public boost::static_visitor<ui::document_view*>
          {
          private:
            type& _proxy;

          public:
            document_view_for_proxy (type& proxy)
              : _proxy (proxy)
            { }

            ui::document_view* operator() (expression_proxy& proxy) const
            {
              return new ui::expression_view (_proxy, proxy.data());
            }

            ui::document_view* operator() (mod_proxy& proxy) const
            {
              return new ui::mod_view (_proxy, proxy.data());
            }

            ui::document_view* operator() (net_proxy& proxy) const
            {
              return new ui::net_view (_proxy, proxy.data(), proxy.display());
            }
          };
        }

        ui::document_view* document_view_factory (type& proxy)
        {
          return boost::apply_visitor (document_view_for_proxy (proxy), proxy);
        }
      }
    }
  }
}
