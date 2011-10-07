// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_UI_EXPRESSION_WIDGET_HPP
#define _PNETE_UI_EXPRESSION_WIDGET_HPP 1

#include <QObject>

#include <pnete/data/proxy.hpp>

#include <pnete/ui/base_editor_widget.hpp>

class QWidget;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class port_lists_widget;

      class expression_widget : public base_editor_widget
      {
        Q_OBJECT;

      public:
        expression_widget ( data::proxy::type& proxy
                          , data::proxy::expression_proxy::data_type& expression
                          , const QStringList& types
                          , QWidget* parent = NULL
                          );

      private slots:
        void name_changed (const QString& name_);

      private:
        data::proxy::expression_proxy::data_type& _expression;
        port_lists_widget* _port_lists;
      };
    }
  }
}

#endif
