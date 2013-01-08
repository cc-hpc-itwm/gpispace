// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UI_EDITOR_WINDOW_HPP
#define _PNETE_UI_EDITOR_WINDOW_HPP 1

#include <pnete/ui/dock_widget.fwd.hpp>

#include <QMainWindow>
#include <QObject>

class QCloseEvent;
class QMenuBar;
class QString;
class QTreeView;
class QUndoView;
class QWidget;

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
      class transition_library_view;

      class editor_window : public QMainWindow
      {
        Q_OBJECT;

      public:
        explicit editor_window (QWidget *parent = NULL);

        void add_transition_library_path ( const QString& path
                                         , bool trusted = false
                                         );

        virtual QMenu* createPopupMenu();
        virtual void closeEvent (QCloseEvent*);

      public slots:
        void slot_new_expression();
        void slot_new_module_call();
        void slot_new_net();
        void open();
        void open (const QString& filename);
        void close_document();
        void quit();

        void update_window_menu();

      private:
        transition_library_view* _transition_library;
        dock_widget* _transition_library_dock;
        view_manager* _view_manager;
        StructureView* _structure_view;
        dock_widget* _structure_view_dock;
        dock_widget* _undo_view_dock;

        QMenu* _windows_menu;

        void setup_menu_and_toolbar();
        void setup_edit_actions (QMenuBar* menu_bar);
        void setup_file_actions (QMenuBar* menu_bar);
        void setup_window_actions (QMenuBar* menu_bar);

        void create_windows (data::internal_type* data);
        QMenu* update_window_menu (QMenu*);

        void readSettings();
        void writeSettings();
      };
    }
  }
}

#endif
