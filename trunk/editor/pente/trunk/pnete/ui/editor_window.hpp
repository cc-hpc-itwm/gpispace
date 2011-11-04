// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UI_EDITOR_WINDOW_HPP
#define _PNETE_UI_EDITOR_WINDOW_HPP 1

#include <QMainWindow>
#include <QObject>

class QString;
class QTreeView;
class QWidget;
class QMenuBar;

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      class internal_type;
    }

    namespace ui
    {
      class view_manager;
      class StructureView;
      class GraphView;
      class transition_library_view;

      namespace graph
      {
        namespace scene { class type; }
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
        void open();
        void open (const QString& filename);
        void close_document();
        void quit();

      private:
        transition_library_view* _transition_library;
        view_manager* _view_manager;
        StructureView* _structure_view;

        void setup_menu_and_toolbar();
        void setup_zoom_actions (QMenuBar* menu_bar);
        void setup_edit_actions (QMenuBar* menu_bar);
        void setup_file_actions (QMenuBar* menu_bar);
        void setup_window_actions (QMenuBar* menu_bar);

        void create_windows (data::internal_type* data);
      };
    }
  }
}

#endif
