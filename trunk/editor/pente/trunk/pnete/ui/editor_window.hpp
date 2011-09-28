// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UI_EDITOR_WINDOW_HPP
#define _PNETE_UI_EDITOR_WINDOW_HPP 1

#include <QMainWindow>
#include <QObject>

class QString;
class QTreeView;
class QWidget;
class QMenuBar;

#include <pnete/data/internal.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class view_manager;
      class StructureView;
      class GraphView;
      namespace graph
      {
        class scene;
      }

      class editor_window : public QMainWindow
      {
        Q_OBJECT;

      public:
        explicit editor_window (QWidget *parent = NULL);

        void set_transition_library_path (const QString& path);
        void add_transition_library_user_path ( const QString& path
                                              , bool trusted = false
                                              );

        virtual QMenu* createPopupMenu();

      public slots:
        void create();
        void save();
        void save (const QString& filename);
        void open();
        void open (const QString& filename);
        void close_document();
        void quit();
        void expand_library();

        void scene_changed (ui::graph::scene*);
        void view_changed (GraphView*);

      private:
        QTreeView* _transition_library;
        view_manager* _view_manager;
        StructureView* _structure_view;

        void setup();
        void setup_menu_and_toolbar();
        void setup_transition_library();
        void setup_structure_view();
        void setup_initial_document();
        void setup_zoom_actions (QMenuBar* menu_bar);
        void setup_edit_actions (QMenuBar* menu_bar);
        void setup_file_actions (QMenuBar* menu_bar);
        void setup_window_actions (QMenuBar* menu_bar);

        void create_windows (data::internal_type::ptr data);
      };
    }
  }
}

#endif
