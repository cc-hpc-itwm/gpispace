#pragma once

#include <QAction>
#include <QList>
#include <QObject>
#include <QPair>
#include <QString>

namespace fhg
{
  namespace pnete
  {
    namespace plugin
    {
      class plugin_base : public QObject
      {
        Q_OBJECT

      public:
        plugin_base (QObject* parent)
          : QObject (parent)
        {}

        //! \note pair.first is name for menu/toolbar, pair.second are contents.
        virtual QList<QPair<QString, QList<QAction*>>> menus() const = 0;
        virtual QList<QPair<QString, QList<QAction*>>> toolbars() const = 0;
      };
    }
  }
}
