// bernd.loerwald@itwm.fraunhofer.de

#ifndef UI_SPLITTABLE_TAB_WIDGET_HPP
#define UI_SPLITTABLE_TAB_WIDGET_HPP 1

#include <QObject>
#include <QTabWidget>

class QMenu;
class QWidget;

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      class Scene;
    }
    namespace ui
    {
      class view_manager;

      class splittable_tab_widget : public QTabWidget
      {
        Q_OBJECT

        private:
          view_manager* _view_manager;
          QMenu* _menu;

        public:
          splittable_tab_widget (view_manager* manager, QWidget* parent = NULL);

        signals:
          void split (splittable_tab_widget*, const Qt::Orientation&);
          void remove (splittable_tab_widget*);

        private slots:
          void split_vertical();
          void split_horizontal();
          void remove_this();
          void tab_close_requested (int index);

        public slots:
          void add_tab_for_scene (graph::Scene* scene);
          void close_current_tab();
      };
    }
  }
}

#endif
