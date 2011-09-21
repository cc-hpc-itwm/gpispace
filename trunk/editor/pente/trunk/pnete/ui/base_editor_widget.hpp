// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UI_BASE_EDITOR_WIDGET_HPP
#define _PNETE_UI_BASE_EDITOR_WIDGET_HPP 1

#include <QObject>
#include <QWidget>

#include <QString>

#include <pnete/data/proxy.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class base_editor_widget : public QWidget
      {
        Q_OBJECT;
      public:
        base_editor_widget ( data::proxy::type& proxy
                           , const QString& fallback_title
                           , QWidget* parent = NULL
                           );

        data::proxy::type& proxy () const;

        QString name () const;

      signals:
        void focus_gained (QWidget*);

      private:
        data::proxy::type& _proxy;

        const QString _fallback_title;
      };
    }
  }
}

#endif
