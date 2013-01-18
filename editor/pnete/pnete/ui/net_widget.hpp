// bernd.loerwald@itwm.fraunhofer.de

#ifndef PNETE_UI_NET_WIDGET_HPP
#define PNETE_UI_NET_WIDGET_HPP

#include <pnete/data/handle/function.hpp>
#include <pnete/data/handle/net.fwd.hpp>

#include <QSplitter>

class QLineEdit;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class net_widget : public QSplitter
      {
        Q_OBJECT;

      public:
        net_widget ( const data::handle::net&
                   , const data::handle::function&
                   , QWidget* parent = NULL
                   );

        void set_name (const QString&);

      private slots:
        void name_edit_changed (const QString&);
        void name_changed
          (const QObject*, const data::handle::function&, const QString&);

      private:
        bool is_my_function (const data::handle::function&);

        data::handle::function _function;
        QLineEdit* _name_edit;
      };
    }
  }
}

#endif
