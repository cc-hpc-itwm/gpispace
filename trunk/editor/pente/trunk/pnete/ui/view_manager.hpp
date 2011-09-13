// bernd.loerwald@itwm.fraunhofer.de

#ifndef UI_VIEW_MANAGER_HPP
#define UI_VIEW_MANAGER_HPP 1

#include <QHash>
#include <QObject>

class QString;
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
      class GraphView;
      class convenience_splitter;
      class splittable_tab_widget;

      class view_manager : public QObject
      {
        Q_OBJECT

        private:
          QHash<QString, graph::Scene*> _open_files;
          convenience_splitter* _splitter;
          splittable_tab_widget* _current_tab_widget;
          //! \note These to depend only on _current_tab_widget but are there for faster access.
          GraphView* _current_view;
          graph::Scene* _current_scene;

        public:
          view_manager (QWidget* parent = NULL);

          splittable_tab_widget* create_default_tab_widget();

          graph::Scene* current_scene() const;
          GraphView* current_view() const;
          splittable_tab_widget* current_tab_widget() const;
          convenience_splitter* splitter() const;

        public slots:
          void focus_changed (QWidget*);

          void create_new_view_for_current_scene();
          void create_new_scene_and_view();

          void zoom_current_view (int);

          void current_scene_add_transition();
          void current_scene_add_place();
          void current_scene_add_struct();

          void create_view_for_file (const QString& filename);
          void save_current_scene (const QString& filename);

        signals:
          void zoomed (int);
          void scene_changed (graph::Scene*);
          void view_changed (GraphView*);
      };
    }
  }
}

#endif
