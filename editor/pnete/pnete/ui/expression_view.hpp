// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_UI_EXPRESSION_VIEW_HPP
#define _PNETE_UI_EXPRESSION_VIEW_HPP 1

#include <QObject>
#include <QString>

#include <pnete/data/proxy.hpp>

#include <pnete/data/handle/expression.fwd.hpp>

#include <pnete/ui/document_view.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class expression_view : public document_view
      {
        Q_OBJECT;

      public:
        expression_view ( data::proxy::type&
                        , const data::handle::expression&
                        , const data::handle::function&
                        );

      private:
        virtual QString fallback_title() const;
      };
    }
  }
}

#endif
