// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_UI_NET_VIEW_HPP
#define _PNETE_UI_NET_VIEW_HPP 1

#include <QObject>

#include <pnete/data/proxy.hpp>

#include <pnete/ui/document_view.hpp>

class QString;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph { class scene; }

      class net_view : public document_view
      {
        Q_OBJECT;

      public:
        net_view ( data::proxy::type&
                 , data::proxy::net_proxy::data_type&
                 , graph::scene*
                 );
      private:
        virtual QString fallback_title() const;
      };
    }
  }
}

#endif
