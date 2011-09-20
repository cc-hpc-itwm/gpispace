// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <boost/variant.hpp>

#include <pnete/ui/GraphScene.hpp>
#include <pnete/ui/document_widget.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace proxy
      {
        const function_type& function (const type & p)
        {
          return boost::apply_visitor (visitor::function(), p);
        }

        namespace visitor
        {
          document_widget_factory::document_widget_factory (type & proxy)
            : _proxy (proxy)
          {}

          ui::document_widget *
          document_widget_factory::operator () (expression_proxy & proxy) const
          {
            return new ui::expression_view ( _proxy
                                           , proxy.data()
                                           );
          }

          ui::document_widget *
          document_widget_factory::operator () (mod_proxy & proxy) const
          {
            return new ui::mod_view ( _proxy
                                    , proxy.data()
                                    );
          }

          ui::document_widget *
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
