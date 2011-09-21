// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UI_DOCUMENT_WIDGET_HPP
#define _PNETE_UI_DOCUMENT_WIDGET_HPP 1

#include <QObject>

#include <pnete/data/proxy.hpp>

#include <pnete/ui/dock_widget.hpp>

class QString;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class base_editor_widget;
      namespace graph
      {
        class scene;
      }

      class document_widget : public dock_widget
      {
        Q_OBJECT;

      public:
        document_widget (const data::proxy::type& proxy);

        base_editor_widget* widget() const;
        void setWidget (base_editor_widget* widget);

      private slots:
        void visibility_changed (bool);
      };

      class expression_view : public document_widget
      {
        Q_OBJECT;

      public:
        expression_view ( data::proxy::type&
                        , data::proxy::expression_proxy::data_type&
                        );
      };

      class mod_view : public document_widget
      {
        Q_OBJECT;

      public:
        mod_view ( data::proxy::type&
                 , data::proxy::mod_proxy::data_type&
                 );
      };

      class net_view : public document_widget
      {
        Q_OBJECT;

      public:
        net_view ( data::proxy::type&
                 , data::proxy::net_proxy::data_type&
                 , graph::scene*
                 );
      };
    }
  }
}

#endif
