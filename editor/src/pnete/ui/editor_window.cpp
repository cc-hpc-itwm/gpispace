// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/editor_window.hpp>

#include <pnete/data/handle/expression.hpp>
#include <pnete/data/handle/module.hpp>
#include <pnete/data/handle/net.hpp>
#include <pnete/data/internal.hpp>
#include <pnete/data/manager.hpp>
#include <pnete/ui/StructureView.hpp>
#include <pnete/ui/TransitionLibraryModel.hpp>
#include <pnete/ui/dock_widget.hpp>
#include <pnete/ui/document_view.hpp>
#include <pnete/ui/execution_monitor.hpp>
#include <pnete/ui/expression_widget.hpp>
#include <pnete/ui/graph/scene.hpp>
#include <pnete/ui/log_monitor.hpp>
#include <pnete/ui/module_call_widget.hpp>
#include <pnete/ui/net_widget.hpp>
#include <pnete/ui/size.hpp>
#include <pnete/ui/transition_library_view.hpp>

#include <fhg/plugin/core/kernel.hpp>
#include <fhg/util/temporary_path.hpp>

#include <sdpa/plugins/sdpactl.hpp>
#include <sdpa/plugins/sdpac.hpp>

#include <util/qt/parent.hpp>

#include <we/loader/loader.hpp>
#include <we/loader/module_call.hpp>
#include <we/mgmt/context.hpp>
#include <we/mgmt/type/activity.hpp>
#include <we/util/token.hpp>

#include <we2/type/value/read.hpp>
#include <we2/type/value/show.hpp>

#include <xml/parse/parser.hpp>

#include <fstream>
#include <sstream>

#include <boost/thread.hpp>

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QFileDialog>
#include <QGridLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QSpinBox>
#include <QString>
#include <QToolBar>
#include <QTreeView>
#include <QUndoGroup>
#include <QUndoView>
#include <QWidget>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      editor_window::editor_window (QWidget* parent)
        : QMainWindow (parent)
        , _transition_library (new transition_library_view (20, 5, this))
        , _transition_library_dock
          (new dock_widget (tr ("library_window"), _transition_library))
        , _structure_view (new StructureView (this))
        , _structure_view_dock
          (new dock_widget (tr ("structure_window"), _structure_view))
        , _undo_group (new QUndoGroup (this))
        , _undo_view_dock
          ( new dock_widget
            (tr ("undo_window"), new QUndoView (_undo_group, this))
          )
        , _windows_menu (NULL)
        , _document_specific_action_menu (NULL)
        , _document_specific_action_toolbar (NULL)
        , _action_save_current_file (NULL)
        , _action_execute_current_file_locally_via_prompt (NULL)
        , _action_execute_current_file_locally_from_file (NULL)
        , _action_execute_current_file_remote_via_prompt (NULL)
      {
        setWindowTitle (tr ("editor_window_title"));

        setDocumentMode (true);
        setDockNestingEnabled (true);
        setTabPosition (Qt::AllDockWidgetAreas, QTabWidget::North);

        addDockWidget (_transition_library_dock);
        addDockWidget (_structure_view_dock);
        addDockWidget (_undo_view_dock);

        setup_menu_and_toolbar();

        _transition_library->setModel (new TransitionLibraryModel (this));

        //! \todo Hand down qApp instead of accessing global state.
        connect ( qApp
                , SIGNAL (focusChanged (QWidget*, QWidget*))
                , SLOT (focus_changed (QWidget*, QWidget*))
                );

        readSettings();
      }

      void editor_window::addDockWidget (QDockWidget* widget)
      {
        QMainWindow::addDockWidget
          (Qt::LeftDockWidgetArea, widget, Qt::Horizontal);
      }

      void editor_window::add_transition_library_path ( const QString& path
                                                      , bool trusted
                                                      )
      {
        qobject_cast<TransitionLibraryModel*> (_transition_library->model())->
            addContentFromDirectory (path, trusted);
      }

      void editor_window::slot_new_expression()
      {
        create_windows ( data::manager::instance()
                       . create (data::internal_type::expression)
                       );
      }
      void editor_window::slot_new_module_call()
      {
        create_windows ( data::manager::instance()
                       . create (data::internal_type::module_call)
                       );
      }
      void editor_window::slot_new_net()
      {
        create_windows ( data::manager::instance()
                       . create (data::internal_type::net)
                       );
      }

      void editor_window::open()
      {
        QString filename (QFileDialog::getOpenFileName
           (this, tr ("Load net"), QDir::homePath(), tr ("XPNET files (*.xpnet);;All (*)")));

        if (filename.isEmpty())
        {
          return;
        }

        create_windows (data::manager::instance().load (filename));
      }

      void editor_window::save_file()
      {
        if (_accessed_widgets.empty())
        {
          return;
        }

        QString filename ( QFileDialog::getSaveFileName
                           ( this
                           , tr ("Save")
                           , QDir::homePath()
                           , tr ("XPNET files (*.xpnet);; All files (*)")
                           )
                         );

        if (filename.isEmpty())
        {
          return;
        }

        if (!filename.endsWith (".xpnet"))
        {
          filename.append (".xpnet");
        }

        data::manager::instance().save
          (_accessed_widgets.top()->function().document(), filename);
      }

      void editor_window::close_document()
      {
        //! \todo Should close all windows of that document!
        current_widget_close();
      }

      void editor_window::quit()
      {
        //! \todo Warn, if unsaved documents open.
        close();
      }

      namespace
      {
        class document_view_for_handle
          : public boost::static_visitor<document_view*>
        {
        private:
          const data::handle::function& _function;

        public:
          document_view_for_handle (const data::handle::function& function)
            : _function (function)
          { }

          document_view* operator() (const data::handle::expression& expr) const
          {
            return new document_view
              ( _function
              , QObject::tr ("<<anonymous expression>>")
              , new expression_widget (expr, _function)
              );
          }

          document_view* operator() (const data::handle::module& module) const
          {
            return new document_view
              ( _function
              , QObject::tr ("<<anonymous module call>>")
              , new module_call_widget (module, _function)
              );
          }

          document_view* operator() (const data::handle::net& net) const
          {
            return new document_view
              ( _function
              , QObject::tr ("<<anonymous net>>")
              , new net_widget (net, _function)
              );
          }
        };

        document_view* document_view_factory
          (const data::handle::function& function)
        {
          //! \todo Why is putting a temporary into apply_visitor impossible?!
          const boost::variant < data::handle::expression
                               , data::handle::module
                               , data::handle::net
                               > content (function.content_handle());
          return boost::apply_visitor
            (document_view_for_handle (function), content);
        }
      }

      void editor_window::create_widget
        (const data::handle::function& function)
      {
        _undo_group->addStack (&function.document()->change_manager());

        document_view* doc_view (document_view_factory (function));

        if (!_accessed_widgets.empty())
        {
          tabifyDockWidget (_accessed_widgets.top(), doc_view);
        }
        else
        {
          addDockWidget (doc_view);
        }

        doc_view->show();
        doc_view->raise();

        foreach (QAction* action, doc_view->actions())
        {
          _document_specific_action_menu->addAction (action);
          _document_specific_action_toolbar->addAction (action);
        }

        _action_save_current_file->setEnabled (true);
        _action_execute_current_file_locally_via_prompt->setEnabled (true);
        _action_execute_current_file_locally_from_file->setEnabled (true);
        _action_execute_current_file_remote_via_prompt->setEnabled (true);
      }

      void editor_window::create_windows (const data::handle::function& function)
      {
        create_widget (function);
        _structure_view->append (function);
      }

      void editor_window::duplicate_active_widget()
      {
        if (!_accessed_widgets.empty())
        {
          create_widget (_accessed_widgets.top()->function());
        }
      }

      void editor_window::current_widget_close()
      {
        if (!_accessed_widgets.empty())
        {
          //! \todo Warn if unsaved changes.
          document_view* current (_accessed_widgets.top());
          foreach (QAction* action, current->actions())
          {
            action->setVisible (false);
          }
          _document_specific_action_menu->menuAction()->setVisible (false);
          _document_specific_action_toolbar->setVisible (false);
          _accessed_widgets.pop();
          removeDockWidget (current);
          delete current;

          if (!_accessed_widgets.empty())
          {
            _accessed_widgets.top()->widget()->setFocus();
          }
          else
          {
            _action_save_current_file->setEnabled (false);
            _action_execute_current_file_locally_via_prompt->setEnabled (false);
            _action_execute_current_file_locally_from_file->setEnabled (false);
            _action_execute_current_file_remote_via_prompt->setEnabled (false);
          }
        }
      }

      void editor_window::focus_changed (QWidget*, QWidget* to_widget)
      {
        document_view* to
          (util::qt::first_parent_being_a<document_view> (to_widget));

        if (!to)
        {
          return;
        }

        if (!_accessed_widgets.empty())
        {
          foreach (QAction* action, _accessed_widgets.top()->actions())
          {
            action->setVisible (false);
          }
        }
        delete _document_specific_action_menu;

        if (_accessed_widgets.contains (to))
        {
          _accessed_widgets.remove (_accessed_widgets.indexOf (to));
        }

        _accessed_widgets.push (to);
        to->function().change_manager().setActive (true);

        _document_specific_action_menu =
          menuBar()->addMenu ("document_specific_actions");
        foreach (QAction* action, to->actions())
        {
          _document_specific_action_menu->addAction (action);
          action->setVisible (true);
        }
        _document_specific_action_menu->menuAction()->setVisible
          (!to->actions().isEmpty());
        _document_specific_action_toolbar->setVisible (!to->actions().isEmpty());
      }

      QMenu* editor_window::createPopupMenu()
      {
        return update_window_menu (new QMenu());
      }

      void editor_window::update_window_menu()
      {
        update_window_menu (_windows_menu);
      }

      QMenu* editor_window::update_window_menu (QMenu* menu)
      {
        menu->clear();

        menu->addAction ( tr ("duplicate_current_window")
                        , this
                        , SLOT (duplicate_active_widget())
                        , QKeySequence::AddTab
                        );
        menu->addAction ( tr ("close_current_window")
                        , this
                        , SLOT (current_widget_close())
                        , QKeySequence::Close
                        );

        menu->addSeparator();

        menu->addAction (_transition_library_dock->toggleViewAction());
        menu->addAction (_structure_view_dock->toggleViewAction());
        menu->addAction (_undo_view_dock->toggleViewAction());

        menu->addSeparator();

        menu->addAction
          (tr ("remote_logging"), this, SLOT (open_remote_logging()));
        menu->addAction
          (tr ("remote_execution"), this, SLOT (open_remote_execution()));

        menu->addSeparator();

        foreach (document_view* view, findChildren<document_view*>())
        {
          menu->addAction (view->toggleViewAction());
        }

        return menu;
      }

      void editor_window::open_remote_logging()
      {
        bool ok;
        const int port ( QInputDialog::getInt ( this
                                              , tr ("port_for_remote_logging")
                                              , tr ("port")
                                              , 2438
                                              , 1024
                                              , 65535
                                              , 1
                                              , &ok
                                              )
                       );
        if (ok)
        {
          dock_widget* widget
            ( new dock_widget ( tr ("log_monitor_for_%1").arg (port)
                              , new log_monitor (port, this)
                              )
            );
          widget->show();
          addDockWidget (widget);
        }
      }
      void editor_window::open_remote_execution()
      {
        bool ok;
        const int port ( QInputDialog::getInt ( this
                                              , tr ("port_for_remote_execution")
                                              , tr ("port")
                                              , 2439
                                              , 1024
                                              , 65535
                                              , 1
                                              , &ok
                                              )
                       );
        if (ok)
        {
          dock_widget* widget
            ( new dock_widget ( tr ("execution_monitor_for_%1").arg (port)
                              , new execution_monitor (port, this)
                              )
            );
          widget->show();
          addDockWidget (widget);
        }
      }

      void editor_window::closeEvent (QCloseEvent* event)
      {
        //! \todo ask the user
        if (/* userReallyWantsToQuit() */ true)
        {
          writeSettings();
          event->accept();
        }
        else
        {
          event->ignore();
        }
      }

      void editor_window::setup_menu_and_toolbar()
      {
        QMenuBar* menu_bar (new QMenuBar (this));
        setMenuBar (menu_bar);

        setUnifiedTitleAndToolBarOnMac (true);

        //! \todo icons for toolbar.
        setup_file_actions (menu_bar);
        setup_edit_actions (menu_bar);
        setup_window_actions (menu_bar);

        _document_specific_action_menu =
          menu_bar->addMenu ("document_specific_actions");
        _document_specific_action_menu->menuAction()->setVisible (false);

        _document_specific_action_toolbar =
          new QToolBar ("document_specific_actions", this);
        _document_specific_action_toolbar->setVisible (false);
        addToolBar (Qt::TopToolBarArea, _document_specific_action_toolbar);
        _document_specific_action_toolbar->setFloatable (false);

        QMenu* runtime_menu (new QMenu (tr ("runtime_menu"), menu_bar));

        QToolBar* runtime_toolbar (new QToolBar (tr ("runtime_toolbar"), this));
        addToolBar (Qt::TopToolBarArea, runtime_toolbar);
        runtime_toolbar->setFloatable (false);

        _action_execute_current_file_locally_via_prompt = runtime_menu->addAction
          ( tr ("execute_locally_input_prompt")
          , this
          , SLOT (execute_locally_inputs_via_prompt())
          );
        _action_execute_current_file_locally_from_file = runtime_menu->addAction
          ( tr ("execute_locally_input_file")
          , this
          , SLOT (execute_locally_inputs_from_file())
          );
        _action_execute_current_file_remote_via_prompt = runtime_menu->addAction
          ( tr ("execute_remote_input_prompt")
          , this
          , SLOT (execute_remote_inputs_via_prompt())
          );
        _action_execute_current_file_locally_via_prompt->setEnabled (false);
        _action_execute_current_file_locally_from_file->setEnabled (false);
        _action_execute_current_file_remote_via_prompt->setEnabled (false);

        runtime_toolbar->addAction
          (_action_execute_current_file_locally_via_prompt);
        runtime_toolbar->addAction
          (_action_execute_current_file_locally_from_file);
        runtime_toolbar->addAction
          (_action_execute_current_file_remote_via_prompt);
      }

      void editor_window::setup_file_actions (QMenuBar* menu_bar)
      {
        QMenu* file_menu (new QMenu (tr ("file_menu"), menu_bar));
        menu_bar->addAction (file_menu->menuAction());

        QToolBar* file_tool_bar (new QToolBar (tr ("file_tool_bar"), this));
        addToolBar (Qt::TopToolBarArea, file_tool_bar);
        file_tool_bar->setFloatable (false);

        QMenu* menu_new (file_menu->addMenu (tr("new")));
        QAction* action_new_expression = menu_new->addAction
          ( tr ("expression")
          , this, SLOT (slot_new_expression())
          , QKeySequence ("Ctrl+E")
          );
        QAction* action_new_module_call = menu_new->addAction
          ( tr ("module_call")
          , this, SLOT (slot_new_module_call())
          , QKeySequence ("Ctrl+M")
          );
        QAction* action_new_net = menu_new->addAction
          (tr ("net"), this, SLOT (slot_new_net()), QKeySequence::New);


        QAction* open_action = file_menu->addAction
          (tr ("open"), this, SLOT (open()), QKeySequence::Open);

        _action_save_current_file = file_menu->addAction
          (tr ("save"), this, SLOT (save_file()), QKeySequence::Save);
        _action_save_current_file->setEnabled (false);

        file_menu->addAction
          (tr ("close_document"), this, SLOT (close_document()));
        file_menu->addAction
          (tr ("quit_application"), this, SLOT (quit()), QKeySequence::Quit);

        file_tool_bar->addAction (action_new_expression);
        file_tool_bar->addAction (action_new_module_call);
        file_tool_bar->addAction (action_new_net);
        file_tool_bar->addAction (open_action);
        file_tool_bar->addAction (_action_save_current_file);
      }

      void editor_window::setup_edit_actions (QMenuBar* menu_bar)
      {
        QMenu* edit_menu (new QMenu (tr ("edit_menu"), menu_bar));
        menu_bar->addAction (edit_menu->menuAction());

        QAction* undo_action
          (_undo_group->createUndoAction (this, tr ("undo_prefix")));
        QAction* redo_action
          (_undo_group->createRedoAction (this, tr ("redo_prefix")));
        undo_action->setShortcuts (QKeySequence::Undo);
        redo_action->setShortcuts (QKeySequence::Redo);
        edit_menu->addAction (undo_action);
        edit_menu->addAction (redo_action);
      }

      void editor_window::setup_window_actions (QMenuBar* menu_bar)
      {
        _windows_menu = menu_bar->addMenu (tr ("windows_menu"));

        connect ( _windows_menu
                , SIGNAL (aboutToShow())
                , SLOT (update_window_menu())
                );

        update_window_menu();
      }

      namespace
      {
        //! \note Context copied from we-eval.
        class eval_context : public we::mgmt::context
        {
        public:
          eval_context (we::loader::loader& module_loader)
            : loader (module_loader)
          { }

          virtual int handle_internally (we::mgmt::type::activity_t& act, net_t&)
          {
            act.inject_input();

            while (act.can_fire())
            {
              we::mgmt::type::activity_t sub (act.extract());
              sub.inject_input();
              sub.execute (this);
              act.inject (sub);
            }

            act.collect_output ();

            return 0;
          }

          virtual int handle_internally (we::mgmt::type::activity_t& act, mod_t& mod)
          {
            module::call (loader, act, mod);

            return 0;
          }

          virtual int handle_internally (we::mgmt::type::activity_t& , expr_t&)
          {
            return 0;
          }

          virtual int handle_externally (we::mgmt::type::activity_t& act, net_t& n)
          {
            return handle_internally (act, n);
          }

          virtual int handle_externally (we::mgmt::type::activity_t& act, mod_t& mod)
          {
            return handle_internally (act, mod);
          }

          virtual int handle_externally (we::mgmt::type::activity_t& act, expr_t& e)
          {
            return handle_internally (act, e);
          }

        private:
          we::loader::loader& loader;
        };

        boost::optional<std::string> get_env (const std::string& name)
        {
          const char *var (getenv (name.c_str()));
          if (var)
          {
            return std::string (var);
          }
          return boost::none;
        }

        std::pair<we::mgmt::type::activity_t, xml::parse::id::ref::function>
          prepare_activity ( const QStack<document_view*>& accessed_widgets
                           , const boost::filesystem::path& temporary_path
                           )
        {
          if (accessed_widgets.empty())
          {
            throw std::runtime_error ("no active widget");
          }

          xml::parse::state::type state;
          state.path_to_cpp() = temporary_path.string();
          boost::optional<std::string> SDPA_HOME (get_env ("SDPA_HOME"));
          if (SDPA_HOME)
          {
            const boost::filesystem::path sdpa_home (*SDPA_HOME);
            const boost::filesystem::path sdpa_include (sdpa_home / "include");
            const boost::filesystem::path sdpa_lib (sdpa_home / "lib");
            state.gen_cxxflags().push_back ( ( boost::format ("-I\"%1%\"")
                                             % sdpa_include
                                             ).str()
                                           );
            state.gen_ldflags().push_back ( ( boost::format ("-L\"%1%\"")
                                            % sdpa_lib
                                            ).str()
                                          );
          }
          //! \todo Add include and link paths

          const xml::parse::id::ref::function function
            (accessed_widgets.top()->function().get().clone());
          xml::parse::post_processing_passes (function, &state);

          xml::parse::generate_cpp (function, state);

          const boost::filesystem::path make_output
            (temporary_path / "MAKEOUTPUT");

          if ( system ( ( "make -C "
                        + temporary_path.string()
                        + " > "
                        + make_output.string()
                        + " 2>&1 "
                        ).c_str()
                      )
             )
          {
            //! \todo process:execute fixen und nutzen
            std::ifstream f (make_output.string().c_str());

            std::stringstream sf;
            while (f.good() && !f.eof())
            {
              sf << (char)f.get();
            }
            throw std::runtime_error (sf.str());
          }

          return std::make_pair
            (xml::parse::xml_to_we (function, state), function);
        }

        bool put_token ( we::mgmt::type::activity_t& activity
                       , const std::string& port_name
                       , const std::string& value
                       )
        {
          try
          {
            try
            {
              we::util::token::put2 ( activity
                                    , port_name
                                    , pnet::type::value::read (value)
                                    );
            }
            catch (const expr::exception::parse::exception& e)
            {
              //! \todo fixed width font
              std::stringstream temp;
              temp << e.what() << std::endl;
              temp << value << std::endl;
              temp << std::string (e.eaten, ' ') << "^" << std::endl;
              throw std::runtime_error (temp.str().c_str());
            }
          }
          catch (const std::runtime_error& e)
          {
            QMessageBox msgBox;
            msgBox.setText (e.what());
            msgBox.setIcon (QMessageBox::Critical);
            msgBox.exec();
            return true;
          }
          return false;
        }

        void show_results_of_activity
          (const we::mgmt::type::activity_t& activity)
        {
          BOOST_FOREACH ( const we::mgmt::type::activity_t::token_on_port_t& top
                        , activity.output()
                        )
          {
            std::stringstream tmp;
            tmp << "on " << activity.transition().get_port (top.second).name()
                << ": " << pnet::type::value::show (top.first);
            QMessageBox msgBox;
            msgBox.setText (QString::fromStdString (tmp.str()));
            msgBox.exec();
          }
        }

        void execute_activity_locally
          ( we::mgmt::type::activity_t activity
          , const boost::filesystem::path& temporary_path
          )
        {
          activity.inject_input();

          we::loader::loader loader;
          loader.append_search_path (temporary_path / "pnetc" / "op");

          eval_context context (loader);

          //! \todo Somehow redirect stdout to buffer
          activity.execute (&context);
          activity.collect_output();

          show_results_of_activity (activity);
        }

        template<typename T>
          T* load_plugin (fhg::core::kernel_t& kernel, const std::string& name)
        {
          if (!kernel.is_plugin_loaded (name))
          {
            kernel.load_plugin (name);
          }

          if (!kernel.is_plugin_loaded (name))
          {
            throw std::runtime_error (name + " failed loading");
          }

          T* plugin (kernel.lookup_plugin_as<T> (name));
          if (!plugin)
          {
            throw std::runtime_error (name + " did not provide correct interface");
          }

          return plugin;
        }
      }

      remote_job_waiting::remote_job_waiting
        (sdpa::Client* client, const std::string& job_id)
          : _client (client)
          , _job_id (job_id)
      { }

      void remote_job_waiting::run()
      {
        std::string status_message;
        int error_code (-1);
        int status (-1);
        while ( (status = _client->status (_job_id, error_code, status_message)) > sdpa::status::CANCELED
              && error_code == 0
              )
        {
          msleep (10);
        }

        if (error_code)
        {
          emit remote_job_failed (_client, QString::fromStdString (_job_id));
        }
        else
        {
          emit remote_job_finished (_client, QString::fromStdString (_job_id));
        }
      }

      void editor_window::remote_job_failed
        (sdpa::Client* client, const QString& job_id)
      {
        QMessageBox msgBox;
        msgBox.setText (job_id + " failed.");
        msgBox.exec();
      }

      void editor_window::remote_job_finished
        (sdpa::Client* client, const QString& job_id_)
      {
        const std::string job_id (job_id_.toStdString());
        std::string output_string;
        client->result (job_id, output_string);
        client->remove (job_id);

        const we::mgmt::type::activity_t output (output_string);
        show_results_of_activity (output);
      }

      namespace
      {
        std::string prompt_for ( const QString& port_name
                               , const boost::optional<std::string>& type
                               , bool* ok
                               )
        {
          if (type == std::string ("file_type"))
          {
            const QString res
              ( QFileDialog::getOpenFileName
                ( NULL
                , QObject::tr ("enter_value_for_input_port_%1").arg (port_name)
                )
              );
            *ok = !res.isNull();
            return res.toStdString();
          }
          else if (type == std::string ("long"))
          {
            return QString ("%1").arg
              ( QInputDialog::getInt
                ( NULL
                , QObject::tr ("value_for_input_token")
                , QObject::tr ("enter_value_for_input_port_%1").arg (port_name)
                , 0
                //! \note These horrible defaults are from Qt.
                , -2147483647
                , 2147483647
                , 1
                , ok
                )
              ).toStdString();
          }
          else if (type == std::string ("double"))
          {
            return QString ("%1").arg
              ( QInputDialog::getDouble
                ( NULL
                , QObject::tr ("value_for_input_token")
                , QObject::tr ("enter_value_for_input_port_%1").arg (port_name)
                , 0
                //! \note These horrible defaults are from Qt.
                , -2147483647
                , 2147483647
                , 1
                , ok
                )
              ).toStdString();
          }
          else
          {
            return QInputDialog::getText
              ( NULL
              , QObject::tr ("value_for_input_token")
              , QObject::tr ("enter_value_for_input_port_%1").arg (port_name)
              , QLineEdit::Normal
              , "[]"
              , ok
              ).toStdString();
          }
        }

        void request_tokens_for_ports
          ( std::pair < we::mgmt::type::activity_t
                      , xml::parse::id::ref::function
                      >* activity_and_fun
          )
        {
          BOOST_FOREACH
            ( const std::string& port_name
            , activity_and_fun->first.transition().port_names (we::type::PORT_IN)
            )
          {
            bool retry (true);
            while (retry)
            {
              const boost::optional<const xml::parse::id::ref::port&> xml_port
                (activity_and_fun->second.get().get_port_in (port_name));

              bool ok;
              const std::string value
                ( prompt_for ( QString::fromStdString (port_name)
                             , xml_port
                             ? xml_port->get().type()
                             : boost::optional<std::string> (boost::none)
                             , &ok
                             )
                );
              if (!ok)
              {
                return;
              }

              retry = put_token (activity_and_fun->first, port_name, value);
            }
          }
        }
      }

      void editor_window::execute_remote_inputs_via_prompt()
      try
      {
        const fhg::util::temporary_path temporary_path;

        std::pair<we::mgmt::type::activity_t, xml::parse::id::ref::function>
          activity_and_fun (prepare_activity (_accessed_widgets, temporary_path));

        request_tokens_for_ports (&activity_and_fun);

        //! \todo Add search path into config (temporarily)!
        // loader.append_search_path (temporary_path / "pnetc" / "op");

        //! \todo Setup plugin search path.
        static fhg::core::kernel_t kernel;

        sdpa::Client* client (load_plugin<sdpa::Client> (kernel, "sdpac"));

        std::string job_id;
        if (client->submit (activity_and_fun.first.to_string(), job_id) == 0)
        {
          remote_job_waiting* waiter (new remote_job_waiting (client, job_id));
          connect ( waiter
                  , SIGNAL (remote_job_failed (sdpa::Client*,QString))
                  , this
                  , SLOT (remote_job_failed (sdpa::Client*,QString))
                  );
          connect ( waiter
                  , SIGNAL (remote_job_finished (sdpa::Client*,QString))
                  , this
                  , SLOT (remote_job_finished (sdpa::Client*,QString))
                  );
          connect (waiter, SIGNAL (finished()), waiter, SLOT (deleteLater()));

          waiter->start();
        }
        else
        {
          QMessageBox msgBox;
          msgBox.setText ("submitting job failed.");
          msgBox.exec();
        }
      }
      catch (const std::runtime_error& e)
      {
        QMessageBox msgBox;
        msgBox.setText (e.what());
        msgBox.setIcon (QMessageBox::Critical);
        msgBox.exec();
      }

      void editor_window::execute_locally_inputs_via_prompt()
      try
      {
        const fhg::util::temporary_path temporary_path;

        std::pair<we::mgmt::type::activity_t, xml::parse::id::ref::function>
          activity_and_fun (prepare_activity (_accessed_widgets, temporary_path));

        request_tokens_for_ports (&activity_and_fun);

        execute_activity_locally (activity_and_fun.first, temporary_path);
      }
      catch (const std::runtime_error& e)
      {
        QMessageBox msgBox;
        msgBox.setText (e.what());
        msgBox.setIcon (QMessageBox::Critical);
        msgBox.exec();
      }

      void editor_window::execute_locally_inputs_from_file()
      try
      {
        const fhg::util::temporary_path temporary_path;

        we::mgmt::type::activity_t activity
          (prepare_activity (_accessed_widgets, temporary_path).first);

        const QString input_filename
          (QFileDialog::getOpenFileName (this, tr ("value_file_for_input")));
        if (input_filename.isEmpty())
        {
          return;
        }

        std::ifstream input_file (input_filename.toStdString().c_str());
        if (!input_file)
        {
          throw std::runtime_error ("bad input file: opening failed");
        }

        std::string input_line;
        while (input_file && getline (input_file, input_line))
        {
          const std::string port_name
            (input_line.substr (0, input_line.find ('=')));
          const std::string value
            (input_line.substr (input_line.find ('=') + 1));

          put_token (activity, port_name, value);
        }

        execute_activity_locally (activity, temporary_path);
      }
      catch (const std::runtime_error& e)
      {
        QMessageBox msgBox;
        msgBox.setText (e.what());
        msgBox.setIcon (QMessageBox::Critical);
        msgBox.exec();
      }


      void editor_window::readSettings()
      {
        QSettings settings;

        settings.beginGroup("MainWindow");
        resize(settings.value("size", QSize(400, 400)).toSize());
        move(settings.value("pos", QPoint(200, 200)).toPoint());
        settings.endGroup();
      }

      void editor_window::writeSettings()
      {
        QSettings settings;

        settings.beginGroup("MainWindow");
        settings.setValue("size", size());
        settings.setValue("pos", pos());
        settings.endGroup();
      }
    }
  }
}
