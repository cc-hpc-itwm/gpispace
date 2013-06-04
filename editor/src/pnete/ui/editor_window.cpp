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
#include <fhg/util/num.hpp>
#include <fhg/util/read_bool.hpp>
#include <fhg/util/temporary_path.hpp>

#include <sdpa/plugins/sdpactl.hpp>
#include <sdpa/plugins/sdpac.hpp>

#include <util/qt/file_line_edit.hpp>
#include <util/qt/parent.hpp>

#include <we/loader/loader.hpp>
#include <we/loader/module_call.hpp>
#include <we/context.hpp>
#include <we/type/activity.hpp>

#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>

#include <xml/parse/parser.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/net.hpp>
#include <xml/parse/type/place.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/transition.hpp>
#include <xml/parse/type/connect.hpp>

#include <fstream>
#include <sstream>

#include <boost/thread.hpp>
#include <boost/unordered_set.hpp>

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QFuture>
#include <QGridLayout>
#include <QGroupBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QSpinBox>
#include <QString>
#include <QtConcurrentRun>
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
        addDockWidget (_undo_view_dock);

        setup_menu_and_toolbar();

        _transition_library->setModel (new TransitionLibraryModel (this));

        //! \todo Hand down qApp instead of accessing global state.
        connect ( qApp
                , SIGNAL (focusChanged (QWidget*, QWidget*))
                , SLOT (focus_changed (QWidget*, QWidget*))
                );

        readSettings();

        setToolButtonStyle (Qt::ToolButtonTextBesideIcon);
      }

      void editor_window::show_transition_library()
      {
        _transition_library_dock->show();
        _transition_library->expandAll();
      }

      void editor_window::show_log_and_execution_monitor (int exec, int log)
      {
        {
          dock_widget* widget
            ( new dock_widget ( tr ("log_monitor_for_%1").arg (log)
                              , new log_monitor (log, this)
                              )
            );
          widget->show();
          addDockWidget (widget);
        }
        {
          dock_widget* widget
            ( new dock_widget ( tr ("execution_monitor_for_%1").arg (exec)
                              , new execution_monitor (exec, this)
                              )
            );
          widget->show();
          addDockWidget (widget);
        }
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
          menuBar()->addMenu (tr ("document_specific_actions"));
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
        menu->addAction (_undo_view_dock->toggleViewAction());

        menu->addSeparator();

        menu->addAction
          (tr ("remote_logging"), this, SLOT (open_remote_logging()));
        menu->addAction
          (tr ("remote_execution"), this, SLOT (open_remote_execution()));

        menu->addSeparator();

        QMenu* toolbars (menu->addMenu (tr ("toolbars_menu")));
        foreach (QToolBar* toolbar, findChildren<QToolBar*>())
        {
          toolbars->addAction (toolbar->toggleViewAction());
        }

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
          menu_bar->addMenu (tr ("document_specific_actions"));
        _document_specific_action_menu->menuAction()->setVisible (false);

        _document_specific_action_toolbar =
          new QToolBar (tr ("document_specific_actions"), this);
        _document_specific_action_toolbar->setVisible (false);
        addToolBar (Qt::TopToolBarArea, _document_specific_action_toolbar);
        _document_specific_action_toolbar->setFloatable (false);

        QMenu* runtime_menu (new QMenu (tr ("runtime_menu"), menu_bar));

        QToolBar* runtime_toolbar (new QToolBar (tr ("runtime_toolbar"), this));
        addToolBar (Qt::TopToolBarArea, runtime_toolbar);
        runtime_toolbar->setFloatable (false);

        _action_execute_current_file_locally_via_prompt = runtime_menu->addAction
          ( QIcon (":/icons/execute.png")
          , tr ("execute_locally_input_prompt")
          , this
          , SLOT (execute_locally_inputs_via_prompt())
          );
        _action_execute_current_file_locally_from_file = runtime_menu->addAction
          ( tr ("execute_locally_input_file")
          , this
          , SLOT (execute_locally_inputs_from_file())
          );
        _action_execute_current_file_remote_via_prompt = runtime_menu->addAction
          ( QIcon (":/icons/execute_remote.png")
          , tr ("execute_remote_input_prompt")
          , this
          , SLOT (execute_remote_inputs_via_prompt())
          );
        _action_execute_current_file_locally_via_prompt->setEnabled (false);
        _action_execute_current_file_locally_from_file->setEnabled (false);
        _action_execute_current_file_remote_via_prompt->setEnabled (false);

        runtime_toolbar->addAction
          (_action_execute_current_file_locally_via_prompt);
        runtime_toolbar->addAction
          (_action_execute_current_file_remote_via_prompt);

        runtime_toolbar->addAction ( QIcon (":/icons/monitor.png")
                                   , tr ("remote_execution")
                                   , this
                                   , SLOT (open_remote_execution())
                                   );

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
          (QIcon (":/icons/new.png"), tr ("net"), this, SLOT (slot_new_net()), QKeySequence::New);


        QAction* open_action = file_menu->addAction
          (QIcon (":/icons/open.png"), tr ("open"), this, SLOT (open()), QKeySequence::Open);

        _action_save_current_file = file_menu->addAction
          (QIcon (":/icons/save.png"), tr ("save"), this, SLOT (save_file()), QKeySequence::Save);
        _action_save_current_file->setEnabled (false);

        file_menu->addAction
          (tr ("close_document"), this, SLOT (close_document()));
        file_menu->addAction
          (tr ("quit_application"), this, SLOT (quit()), QKeySequence::Quit);

        file_tool_bar->addAction (action_new_net);
        file_tool_bar->addAction (open_action);
        file_tool_bar->addAction (_action_save_current_file);
      }

      void editor_window::setup_edit_actions (QMenuBar* menu_bar)
      {
        QMenu* edit_menu (new QMenu (tr ("edit_menu"), menu_bar));
        menu_bar->addAction (edit_menu->menuAction());

        QToolBar* edit_tool_bar (new QToolBar (tr ("edit_tool_bar"), this));
        addToolBar (Qt::TopToolBarArea, edit_tool_bar);
        edit_tool_bar->setFloatable (false);

        QAction* undo_action
          (_undo_group->createUndoAction (this, tr ("undo_prefix")));
        QAction* redo_action
          (_undo_group->createRedoAction (this, tr ("redo_prefix")));
        undo_action->setIcon (QIcon (":/icons/undo.png"));
        redo_action->setIcon (QIcon (":/icons/redo.png"));
        undo_action->setIconText (tr ("undo"));
        redo_action->setIconText (tr ("redo"));
        undo_action->setShortcuts (QKeySequence::Undo);
        redo_action->setShortcuts (QKeySequence::Redo);
        edit_menu->addAction (undo_action);
        edit_menu->addAction (redo_action);

        edit_tool_bar->addAction (undo_action);
        edit_tool_bar->addAction (redo_action);
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
        class eval_context : public we::context
        {
          boost::mt19937 _engine;

        public:
          eval_context (we::loader::loader& module_loader)
            : loader (module_loader)
          { }

          virtual void handle_internally (we::type::activity_t& act, net_t const&)
          {
            if (act.transition().net())
            {
              while ( boost::optional<we::type::activity_t> sub
                    = boost::get<we::type::net_type&> (act.transition().data())
                    . fire_expressions_and_extract_activity_random (_engine)
                    )
              {
                sub->execute (this);
                act.inject (*sub);
              }
            }
          }

          virtual void handle_internally (we::type::activity_t& act, mod_t const& mod)
          {
            //!\todo pass a real gspc::drts::context here
            we::loader::module_call (loader, 0, act, mod);
          }

          virtual void handle_internally (we::type::activity_t& , expr_t const&)
          {
          }

          virtual void handle_externally (we::type::activity_t& act, net_t const& n)
          {
            handle_internally (act, n);
          }

          virtual void handle_externally (we::type::activity_t& act, mod_t const& mod)
          {
            handle_internally (act, mod);
          }

          virtual void handle_externally (we::type::activity_t& act, expr_t const& e)
          {
            handle_internally (act, e);
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

        bool put_token ( we::type::activity_t& activity
                       , const std::string& port_name
                       , const std::string& value
                       )
        {
          try
          {
            try
            {
              activity.add_input
                ( activity.transition().input_port_by_name (port_name)
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

        namespace
        {
          we::type::PortDirection fake_dir
            (const xml::parse::type::port_type& port)
          {
            if (port.direction() == we::type::PORT_TUNNEL)
            {
              const boost::optional<std::string> tunnel_direction
                (port.properties().get ("fhg.pnete.tunnel.direction"));

              return !tunnel_direction ? we::type::PORT_IN
                : *tunnel_direction == "out" ? we::type::PORT_OUT
                : *tunnel_direction == "in" ? we::type::PORT_IN
                : throw std::runtime_error
                  ("bad fhg.pnete.tunnel.direction (neither 'in' nor 'out')");
            }
            else
            {
              return port.direction();
            }
          }
        }

        namespace
        {
          boost::unordered_set<std::string> init_typenames()
          {
            boost::unordered_set<std::string> tns;

            tns.insert ("file_type");
            tns.insert ("function_type");

            return tns;
          }

          boost::optional<std::runtime_error> invoke_make
            (const boost::filesystem::path& temporary_path)
          {
            const boost::filesystem::path make_output
              (temporary_path / "MAKEOUTPUT");

            const boost::optional<std::string> HOME (get_env ("HOME"));

            if (!HOME)
            {
              return std::runtime_error ("$HOME not set!?");
            }

            if ( system ( ( "make -C "
                          + temporary_path.string()
                          + " LIB_DESTDIR=" + *HOME + "/.sdpa/modules install > "
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
              return std::runtime_error (sf.str());
            }

            return boost::none;
          }
        }

        std::pair<we::type::activity_t, xml::parse::id::ref::function>
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
            const boost::filesystem::path sdpa_libexec (sdpa_home / "libexec" / "sdpa");
            state.gen_cxxflags().push_back ( ( boost::format ("-I\"%1%\"")
                                             % sdpa_include
                                             ).str()
                                           );
            state.gen_ldflags().push_back ( ( boost::format ("-L\"%1%\"")
                                            % sdpa_lib
                                            ).str()
                                          );
            state.gen_ldflags().push_back ( ( boost::format ("-L\"%1%\"")
                                            % sdpa_libexec
                                            ).str()
                                          );
          }
          //! \todo Add include and link paths

          const xml::parse::id::ref::function function
            (accessed_widgets.top()->function().get().clone());

          const xml::parse::id::ref::net net (*function.get().get_net());

          int slot_gen_count (0);

          foreach ( const xml::parse::id::ref::transition& trans
                  , net.get().transitions().ids()
                  )
          {
            const xml::parse::id::ref::function fun
              (trans.get().resolved_function());

            const std::string gens
              ( fun.get().properties().get ("fhg.seislib.slot.num.generator")
              .get_value_or ("0")
              );
            fhg::util::parse::position_string pos (gens);
            slot_gen_count += fhg::util::read_int (pos);

            foreach ( const xml::parse::id::ref::port& pid
                    , fun.get().ports().ids()
                    )
            {
              xml::parse::type::port_type& port (pid.get_ref());

              const std::string name (trans.get().name() + "__xxx__" + port.name());

              bool has_any_connection (false);
              foreach ( const xml::parse::type::connect_type& connect
                      , trans.get().connections().values()
                      )
              {
                if (connect.port() == port.name())
                {
                  has_any_connection = true;
                  break;
                }
              }
              foreach ( const xml::parse::type::place_map_type& place_map
                      , trans.get().place_map().values()
                      )
              {
                if (place_map.place_virtual() == port.name())
                {
                  has_any_connection = true;
                  break;
                }
              }
              if (!has_any_connection)
              {
                //! \todo which types?
                static boost::unordered_set<std::string> tns (init_typenames());

                if (tns.find (port.type()) == tns.end())
                {
                  throw std::runtime_error (std::string ("unconnected port ") + port.name() + " with non-auto-connectable type " + port.type());
                }

                net.get_ref().push_place
                  ( ::xml::parse::type::place_type
                    ( net.id_mapper()->next_id()
                    , net.id_mapper()
                    , boost::none
                    , XML_PARSE_UTIL_POSITION_GENERATED()
                    , name
                    , port.type()
                    , boost::none
                    ).make_reference_id()
                  );

                function.get_ref().push_port
                  ( ::xml::parse::type::port_type
                    ( function.id_mapper()->next_id()
                    , function.id_mapper()
                    , boost::none
                    , XML_PARSE_UTIL_POSITION_GENERATED()
                    , name
                    , port.type()
                    , name
                    , fake_dir (port)
                    ).make_reference_id()
                  );

                if (port.direction() == we::type::PORT_TUNNEL)
                {
                  trans.get_ref().push_place_map
                    ( ::xml::parse::type::place_map_type
                      ( trans.id_mapper()->next_id()
                      , trans.id_mapper()
                      , boost::none
                      , XML_PARSE_UTIL_POSITION_GENERATED()
                      , port.name()
                      , name
                      , we::type::property::type()
                      ).make_reference_id()
                    );
                }
                else
                {
                  trans.get_ref().push_connection
                    ( ::xml::parse::type::connect_type
                      ( trans.id_mapper()->next_id()
                      , trans.id_mapper()
                      , boost::none
                      , XML_PARSE_UTIL_POSITION_GENERATED()
                      , name
                      , port.name()
                      , port.direction() == we::type::PORT_IN
                      ? we::edge::PT
                      : we::edge::TP
                      ).make_reference_id()
                    );
                }
              }
            }
          }

          xml::parse::post_processing_passes (function, &state);

          xml::parse::generate_cpp (function, state);

          {
            QMessageBox box (QMessageBox::Information, "Compiling modules", "Please wait..", QMessageBox::NoButton);
            box.setStandardButtons (0);
            box.show();

            const QFuture<boost::optional<std::runtime_error> > compilation
              (QtConcurrent::run (invoke_make, temporary_path));

            while (!compilation.isFinished())
            {
              qApp->processEvents();
            }

            box.hide();
            qApp->processEvents();

            if (compilation.result())
            {
              throw *compilation.result();
            }
          }

          we::type::activity_t activity
            (xml::parse::xml_to_we (function, state));

          put_token (activity, "REFLECT_seislib_slot_num_generator", (boost::format ("%1%") % slot_gen_count).str());

          return std::make_pair (activity, function);
        }

        void show_results_of_activity
          (const we::type::activity_t& activity)
        {
          BOOST_FOREACH ( const we::type::activity_t::token_on_port_t& top
                        , activity.output()
                        )
          {
            std::stringstream tmp;
            tmp << "on " << activity.transition().ports_output().at (top.second).name()
                << ": " << pnet::type::value::show (top.first);
            QMessageBox msgBox;
            msgBox.setText (QString::fromStdString (tmp.str()));
            msgBox.exec();
          }
        }

        void execute_activity_locally
          ( we::type::activity_t activity
          , const boost::filesystem::path& temporary_path
          )
        {
          we::loader::loader loader;
          loader.append_search_path (temporary_path / "pnetc" / "op");

          eval_context context (loader);

          //! \todo Somehow redirect stdout to buffer
          activity.execute (&context);

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

        const we::type::activity_t output (output_string);
        show_results_of_activity (output);
      }

      namespace
      {
        int string_to_int (const QString& str)
        {
          const std::string stdstr (str.toStdString());
          fhg::util::parse::position_string pos (stdstr);
          return fhg::util::read_int (pos);
        }

        QString checkbox_to_string (const QCheckBox* box)
        {
          return box->isChecked() ? "true" : "false";
        }
        QString spinbox_to_string (const QSpinBox* box)
        {
          return QString ("%1").arg (box->value());
        }
        QString spinbox_to_memory_size_type (const QSpinBox* box)
        {
          return QString ("[size:=%1L]")
            .arg ((1UL << 20) * static_cast<long> (box->value()));
        }
        QString file_line_edit_to_file_type (const util::qt::file_line_edit* edit)
        {
          return QString ("[name:=\"%1\",type:=\"raw\"]").arg (edit->text());
        }
        QString file_line_edit_to_function_type (const util::qt::file_line_edit* edit)
        {
          return QString ("[binary:=\"%1\"]").arg (edit->text());
        }

        std::pair<QWidget*, boost::function<QString()> > widget_for_item
          (const std::string& type, const boost::optional<QString>& def)
        {
          if (type == "bool")
          {
            QCheckBox* box (new QCheckBox);
            box->setChecked ( fhg::util::read_bool
                              (def.get_value_or ("false").toStdString())
                            );
            return std::pair<QWidget*, boost::function<QString()> >
              (box, boost::bind (checkbox_to_string, box));
          }
          else if (type == "file_type")
          {
            util::qt::file_line_edit* edit
              ( new util::qt::file_line_edit
                (QFileDialog::AnyFile, def.get_value_or (""))
              );
            return std::pair<QWidget*, boost::function<QString()> >
              (edit, boost::bind (file_line_edit_to_file_type, edit));
          }
          else if (type == "function_type")
          {
            util::qt::file_line_edit* edit
              ( new util::qt::file_line_edit
                (QFileDialog::AnyFile, def.get_value_or (""))
              );
            return std::pair<QWidget*, boost::function<QString()> >
              (edit, boost::bind (file_line_edit_to_function_type, edit));
          }
          else if (type == "memory_size_type")
          {
            QSpinBox* edit (new QSpinBox);
            edit->setMinimum (0);
            edit->setMaximum (INT_MAX);
            edit->setValue (string_to_int (def.get_value_or ("0")));
            edit->setSuffix (" MiB");
            return std::pair<QWidget*, boost::function<QString()> >
              (edit, boost::bind (spinbox_to_memory_size_type, edit));
          }
          else if (type == "long")
          {
            QSpinBox* edit (new QSpinBox);
            edit->setMinimum (INT_MIN);
            edit->setMaximum (INT_MAX);
            edit->setValue (string_to_int (def.get_value_or ("0")));
            return std::pair<QWidget*, boost::function<QString()> >
              (edit, boost::bind (spinbox_to_string, edit));
          }
          else if (type == "string")
          {
            QLineEdit* edit (new QLineEdit (def.get_value_or ("")));
            return std::pair<QWidget*, boost::function<QString()> >
              (edit, boost::bind (&QLineEdit::text, edit));
          }
          else
          {
            QLineEdit* edit (new QLineEdit (def.get_value_or ("[]")));
            return std::pair<QWidget*, boost::function<QString()> >
              (edit, boost::bind (&QLineEdit::text, edit));
          }
        }

        boost::optional<QString>
          opt_std_to_qstring (const boost::optional<const std::string&>& in)
        {
          return in ? QString::fromStdString (*in) : boost::optional<QString>();
        }

        template<typename T> T sort (T t)
        {
          qSort (t);
          return t;
        }

        void request_tokens_for_ports
          ( std::pair < we::type::activity_t
                      , xml::parse::id::ref::function
                      >* activity_and_fun
          )
        {
          QMap<QString, boost::function<QString()> > value_getters;

          QDialog* dialog (new QDialog);
          dialog->setWindowTitle ("Please enter input values");
          new QVBoxLayout (dialog);

          QMap<boost::optional<QString>, QStringList> port_groups;

          BOOST_FOREACH
            ( const we::type::port_t& port
            , activity_and_fun->first.transition().ports_input()
            | boost::adaptors::map_values
            )
          {
            {
              const QString port_name (QString::fromStdString (port.name()));

              const QStringList parts (port_name.split ("__xxx__"));
              if (parts.size() == 2)
              {
                port_groups[parts[0]].append (parts[1]);
              }
              else
              {
                port_groups[boost::none].append (port_name);
              }
            }

            foreach (const boost::optional<QString>& group, sort (port_groups.keys()))
            {
              QGroupBox* box (new QGroupBox (group.get_value_or ("Global")));

              dialog->layout()->addWidget (box);

              QFormLayout* box_layout (new QFormLayout (box));

              BOOST_FOREACH (const QString& val, sort (port_groups[group]))
              {
                const QString port_name (group ? *group + "__xxx__" + val : val);

                if (port_name.startsWith ("REFLECT_"))
                {
                  continue;
                }

                const boost::optional<const xml::parse::id::ref::port&> xml_port
                  (activity_and_fun->second.get().get_port_in (port_name.toStdString()));

                const std::pair<QWidget*, boost::function<QString()> > ret
                  ( widget_for_item ( xml_port->get().type()
                                    , opt_std_to_qstring
                                      ( xml_port->get().properties().get
                                        ("fhg.pnete.port.default")
                                      )
                                    )
                  );
                box_layout->addRow (val, ret.first);
                value_getters[port_name] = ret.second;
              }
            }

            QDialogButtonBox* buttons
              ( new QDialogButtonBox ( QDialogButtonBox::Abort | QDialogButtonBox::Ok
                                     , Qt::Horizontal
                                     , dialog
                                     )
              );

            dialog->connect (buttons, SIGNAL (accepted()), SLOT (accept()));
            dialog->connect (buttons, SIGNAL (rejected()), SLOT (reject()));

            dialog->layout()->addWidget (buttons);

            if (!value_getters.empty() && !dialog->exec())
            {
              return;
            }

            QMap<QString, boost::function<QString()> >::const_iterator i
              (value_getters.constBegin());
            while (i != value_getters.constEnd())
            {
              put_token ( activity_and_fun->first
                        , i.key().toStdString()
                        , i.value()().toStdString()
                        );
              ++i;
            }
          }
        }
      }

      void editor_window::execute_remote_inputs_via_prompt()
      try
      {
        const fhg::util::temporary_path temporary_path;

        std::pair<we::type::activity_t, xml::parse::id::ref::function>
          activity_and_fun (prepare_activity (_accessed_widgets, temporary_path));

        request_tokens_for_ports (&activity_and_fun);

        //! \todo Add search path into config (temporarily)!
        // loader.append_search_path (temporary_path / "pnetc" / "op");

        //! \todo Setup plugin search path.
        static fhg::core::kernel_t kernel;

        const boost::optional<std::string> SDPA_HOME (get_env ("SDPA_HOME"));
        if (!SDPA_HOME)
        {
          throw std::runtime_error ("SDPA_HOME not set");
        }

        kernel.add_search_path (*SDPA_HOME + "/libexec/fhg/plugins/");

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

        std::pair<we::type::activity_t, xml::parse::id::ref::function>
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

        we::type::activity_t activity
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
