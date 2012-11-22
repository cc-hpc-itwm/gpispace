// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/data/proxy.hpp>

#include <pnete/data/handle/expression.hpp>
#include <pnete/data/internal.hpp>
#include <pnete/ui/document_view.hpp>
#include <pnete/ui/expression_view.hpp>
#include <pnete/ui/mod_view.hpp>
#include <pnete/ui/net_view.hpp>

#include <boost/variant.hpp>

#include <QObject>

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
            : public boost::static_visitor<const handle::function&>
          {
          public:
            template<typename T>
              const handle::function& operator() (const T& x) const
            {
              return x.function();
            }
          };
        }

        handle::function function (const type& proxy)
        {
          return boost::apply_visitor (visitor_function(), proxy);
        }

        internal_type* root (const type& proxy)
        {
          return boost::apply_visitor (visitor_root(), proxy);
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
              return new ui::expression_view
                ( _proxy, handle::expression ( proxy.data()
                                             , root (_proxy)->change_manager()
                                             )
                );
            }

            ui::document_view* operator() (mod_proxy& proxy) const
            {
              return new ui::mod_view (_proxy, proxy.data());
            }

            ui::document_view* operator() (net_proxy& proxy) const
            {
              return new ui::net_view (_proxy, proxy.display());
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
