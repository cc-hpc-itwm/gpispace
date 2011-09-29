// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/data/proxy.hpp>

#include <QObject>

#include <boost/variant.hpp>

#include <pnete/ui/document_widget.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace proxy
      {
        const function_type& function (const type& proxy)
        {
          return boost::apply_visitor (visitor::function<const function_type&>(), proxy);
        }
        function_type& function (type& proxy)
        {
          return boost::apply_visitor (visitor::function<function_type&> (), proxy);
        }

        ::fhg::pnete::data::internal_type::ptr root (const type& proxy)
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

          mod_type::mod_type ( xml_type::mod_type& mod
                             , xml_type::ports_type& in
                             , xml_type::ports_type& out
                             )
            : _mod (mod)
            , _in (in)
            , _out (out)
          {}
          xml_type::mod_type& mod_type::mod () { return _mod; }
          xml_type::ports_type& mod_type::in () { return _in; }
          xml_type::ports_type& mod_type::out () { return _out; }

          net_type::net_type ( xml_type::net_type& net
                             )
            : _net (net)
          {}
          xml_type::net_type& net_type::net () { return _net; }
        }

        ui::document_widget* document_widget_factory (type& proxy)
        {
          return boost::apply_visitor
            (visitor::document_widget_factory (proxy), proxy);
        }

        namespace visitor
        {
          document_widget_factory::document_widget_factory (type & proxy)
            : _proxy (proxy)
          {}

          ui::document_widget*
          document_widget_factory::operator () (expression_proxy & proxy) const
          {
            return new ui::expression_view ( _proxy
                                           , proxy.data()
                                           );
          }

          ui::document_widget*
          document_widget_factory::operator () (mod_proxy & proxy) const
          {
            return new ui::mod_view ( _proxy
                                    , proxy.data()
                                    );
          }

          ui::document_widget*
          document_widget_factory::operator () (net_proxy & proxy) const
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
