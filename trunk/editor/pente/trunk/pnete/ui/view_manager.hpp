// bernd.loerwald@itwm.fraunhofer.de

#ifndef UI_VIEW_MANAGER_HPP
#define UI_VIEW_MANAGER_HPP 1

#include <QObject>

#include <pnete/data/internal.hpp>
#include <pnete/util.hpp>

#include <boost/unordered_map.hpp>

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
      class dockable_graph_view;
      class editor_window;
      class GraphView;

      class view_manager : public QObject
      {
        Q_OBJECT

        public:
          view_manager (editor_window* parent);

        public slots:
          void focus_changed (QWidget*);

          void current_view_zoom (int);
          void current_view_zoom_in();
          void current_view_zoom_out();
          void current_view_reset_zoom();

          void current_scene_add_transition();
          void current_scene_add_place();
          void current_scene_add_struct();
          void current_scene_auto_layout();

          void create_new_view_for_current_scene();
          void create_view (data::internal::ptr data);
          void save_current_scene (const QString& filename);
          void current_document_close();

        signals:
          void zoomed (int);
          void scene_changed (graph::Scene*);
          void view_changed (GraphView*);

        private:
          editor_window* _editor_window;

          typedef boost::unordered_map
                  < data::internal::ptr
                  , graph::Scene*
                  , util::ptr_hasher<data::internal::ptr::value_type>
                  > scenes_type;

          scenes_type _scenes;

          QList<dockable_graph_view*> _accessed_widgets;
          GraphView* _current_view;
          graph::Scene* _current_scene;

          void add_on_top_of_current_widget (dockable_graph_view* w);
      };
    }
  }
}

#endif
