// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <pnete/ui/dock_widget.fwd.hpp>

#include <pnete/data/handle/function.fwd.hpp>
#include <pnete/data/manager.fwd.hpp>
#include <pnete/ui/document_view.fwd.hpp>

#include <fhg/util/dl.hpp>

#include <QMainWindow>
#include <QObject>
#include <QStack>
#include <QThread>

#include <list>

class QCloseEvent;
class QMenuBar;
class QString;
class QTreeView;
class QUndoGroup;
class QUndoView;
class QWidget;

namespace sdpa
{
  namespace client
  {
    class Client;
  }
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
        remote_job_waiting (sdpa::client::Client*, const std::string&);

      protected:
        virtual void run() override;

      signals:
        void remote_job_finished (sdpa::client::Client*, const QString&);
        void remote_job_failed (sdpa::client::Client*, const QString&);

      private:
        sdpa::client::Client* _client;
        std::string _job_id;
      };

      class editor_window : public QMainWindow
      {
        Q_OBJECT;

      public:
        editor_window ( data::manager&
                      , std::list<util::scoped_dlhandle> const& plugins
                      , QWidget *parent = nullptr
                      );

        void add_transition_library_path (const QString&, bool trusted = false);

        virtual QMenu* createPopupMenu() override;

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

        void remote_job_finished (sdpa::client::Client*, const QString&);
        void remote_job_failed (sdpa::client::Client*, const QString&);

      protected:
        virtual void closeEvent (QCloseEvent*) override;

      private:
        data::manager& _data_manager;

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

        void setup_menu_and_toolbar
          (std::list<util::scoped_dlhandle> const& plugins);
        void setup_edit_actions (QMenuBar* menu_bar);
        void setup_file_actions (QMenuBar* menu_bar);
        void setup_window_actions (QMenuBar* menu_bar);

        void create_windows (const data::handle::function&);

        QMenu* update_window_menu (QMenu*);

        void readSettings();
        void writeSettings();

        void addDockWidget (QDockWidget*);
      };
    }
  }
}
