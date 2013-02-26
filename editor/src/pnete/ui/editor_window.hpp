// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UI_EDITOR_WINDOW_HPP
#define _PNETE_UI_EDITOR_WINDOW_HPP 1

#include <pnete/ui/dock_widget.fwd.hpp>

#include <pnete/data/handle/function.fwd.hpp>
#include <pnete/ui/document_view.fwd.hpp>

#include <QMainWindow>
#include <QObject>
#include <QStack>
#include <QThread>

class QCloseEvent;
class QMenuBar;
class QString;
class QTreeView;
class QUndoGroup;
class QUndoView;
class QWidget;

namespace sdpa
{
  class Client;
}

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class StructureView;
      class transition_library_view;

      class remote_job_waiting : public QThread
      {
        Q_OBJECT;

      public:
        remote_job_waiting (sdpa::Client*, const std::string&);

      protected:
        virtual void run();

      signals:
        void remote_job_finished (sdpa::Client*, const QString&);
        void remote_job_failed (sdpa::Client*, const QString&);

      private:
        sdpa::Client* _client;
        std::string _job_id;
      };

      class editor_window : public QMainWindow
      {
        Q_OBJECT;

      public:
        explicit editor_window (QWidget *parent = NULL);

        void add_transition_library_path (const QString&, bool trusted = false);

        virtual QMenu* createPopupMenu();

      public slots:
        void slot_new_expression();
        void slot_new_module_call();
        void slot_new_net();

        void open();
        void save_file();

        void close_document();
        void quit();

        void create_widget (const data::handle::function&);
        void duplicate_active_widget();
        void current_widget_close();
        void focus_changed (QWidget*, QWidget*);
        void update_window_menu();

        void execute_locally_inputs_via_prompt();
        void execute_locally_inputs_from_file();
        void execute_remote_inputs_via_prompt();

        void open_remote_logging();
        void open_remote_execution();

        void remote_job_finished (sdpa::Client*, const QString&);
        void remote_job_failed (sdpa::Client*, const QString&);

      protected:
        virtual void closeEvent (QCloseEvent*);

      private:
        transition_library_view* _transition_library;
        dock_widget* _transition_library_dock;
        StructureView* _structure_view;
        dock_widget* _structure_view_dock;
        QUndoGroup* _undo_group;
        dock_widget* _undo_view_dock;

        QMenu* _windows_menu;
        QMenu* _document_specific_action_menu;
        QToolBar* _document_specific_action_toolbar;
        QAction* _action_save_current_file;
        QAction* _action_execute_current_file_locally_via_prompt;
        QAction* _action_execute_current_file_locally_from_file;
        QAction* _action_execute_current_file_remote_via_prompt;

        QStack<document_view*> _accessed_widgets;

        void setup_menu_and_toolbar();
        void setup_edit_actions (QMenuBar* menu_bar);
        void setup_file_actions (QMenuBar* menu_bar);
        void setup_window_actions (QMenuBar* menu_bar);

        void create_windows (const data::handle::function&);

        QMenu* update_window_menu (QMenu*);

        void readSettings();
        void writeSettings();
      };
    }
  }
}

#endif
