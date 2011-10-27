// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_UI_MOD_VIEW_HPP
#define _PNETE_UI_MOD_VIEW_HPP 1

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
      class mod_view : public document_view
      {
        Q_OBJECT;

      public:
        mod_view ( data::proxy::type&
                 , data::proxy::mod_proxy::data_type&
                 );

      private:
        virtual QString fallback_title() const;
      };
    }
  }
}

#endif
