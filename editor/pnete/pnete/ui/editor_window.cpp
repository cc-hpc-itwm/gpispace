// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/editor_window.hpp>

#include <pnete/data/internal.hpp>
#include <pnete/data/manager.hpp>
#include <pnete/ui/StructureView.hpp>
#include <pnete/ui/TransitionLibraryModel.hpp>
#include <pnete/ui/dock_widget.hpp>
#include <pnete/ui/document_view.hpp>
#include <pnete/ui/expression_widget.hpp>
#include <pnete/ui/graph/scene.hpp>
#include <pnete/ui/graph_view.hpp>
#include <pnete/ui/module_call_widget.hpp>
#include <pnete/ui/size.hpp>
#include <pnete/ui/transition_library_view.hpp>
#include <pnete/weaver/display.hpp>

#include <util/qt/parent.hpp>

#include <we/loader/loader.hpp>
#include <we/loader/module_call.hpp>
#include <we/mgmt/context.hpp>
#include <we/mgmt/type/activity.hpp>
#include <we/type/value/read.hpp>
#include <we/util/token.hpp>

#include <xml/parse/parser.hpp>

#include <fstream>
#include <sstream>

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
      static const Qt::DockWidgetArea dock_position (Qt::LeftDockWidgetArea);

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
        , _action_save_current_file (NULL)
        , _action_execute_current_file_locally_via_prompt (NULL)
        , _action_execute_current_file_locally_from_file (NULL)
      {
        setWindowTitle (tr ("editor_window_title"));

        setDocumentMode (true);
        setDockNestingEnabled (true);
        setTabPosition (Qt::AllDockWidgetAreas, QTabWidget::North);

        addDockWidget (dock_position, _transition_library_dock, Qt::Horizontal);
        addDockWidget (dock_position, _structure_view_dock, Qt::Horizontal);
        addDockWidget (dock_position, _undo_view_dock, Qt::Horizontal);

        setup_menu_and_toolbar();

        _transition_library->setModel (new TransitionLibraryModel (this));

        //! \todo Hand down qApp instead of accessing global state.
        connect ( qApp
                , SIGNAL (focusChanged (QWidget*, QWidget*))
                , SLOT (focus_changed (QWidget*, QWidget*))
                );

        readSettings();
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
                       . create(data::internal_type::expression)
                       );
      }
      void editor_window::slot_new_module_call()
      {
        create_windows ( data::manager::instance()
                       . create(data::internal_type::module_call)
                       );
      }
      void editor_window::slot_new_net()
      {
        create_windows ( data::manager::instance()
                       . create(data::internal_type::net)
                       );
      }

      void editor_window::open()
      {
        QString filename (QFileDialog::getOpenFileName
           (this, tr ("Load net"), QDir::homePath(), tr ("XML files (*.xml);;All (*)")));

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
                           , tr ("XML files (*.xml);; All files (*)")
                           )
                         );

        if (filename.isEmpty())
        {
          return;
        }

        if (!filename.endsWith (".xml"))
        {
          filename.append (".xml");
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
        using namespace data::proxy;

        class document_view_for_proxy
          : public boost::static_visitor<document_view*>
        {
        private:
          const data::handle::function& _function;

        public:
          document_view_for_proxy (const data::handle::function& function)
            : _function (function)
          { }

          document_view* operator() (const expression_proxy& proxy) const
          {
            return new document_view
              ( _function
              , QObject::tr ("<<anonymous expression>>")
              , new expression_widget (proxy.data(), _function)
              );
          }

          document_view* operator() (const mod_proxy& proxy) const
          {
            return new document_view
              ( _function
              , QObject::tr ("<<anonymous module call>>")
              , new module_call_widget (proxy.data(), _function)
              );
          }

          document_view* operator() (const net_proxy& proxy) const
          {
            return new document_view
              ( _function
              , QObject::tr ("<<anonymous net>>")
              , new graph_view (proxy.display())
              );
          }
        };

        document_view* document_view_factory
          (const data::handle::function& function)
        {
          const data::proxy::type proxy
            (weaver::display::function (function.id(), function.document()));
          return boost::apply_visitor
            (document_view_for_proxy (function), proxy);
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
          addDockWidget (Qt::LeftDockWidgetArea, doc_view, Qt::Horizontal);
        }

        doc_view->show();
        doc_view->raise();

        foreach (QAction* action, doc_view->actions())
        {
          _document_specific_action_menu->addAction (action);
        }

        _action_save_current_file->setEnabled (true);
        _action_execute_current_file_locally_via_prompt->setEnabled (true);
        _action_execute_current_file_locally_from_file->setEnabled (true);
      }

      void editor_window::create_windows (data::internal_type* data)
      {
        create_widget (data::handle::function (data->function(), data));
        _structure_view->append (data::handle::function (data->function(), data));
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

        if (_accessed_widgets.contains (to))
        {
          _accessed_widgets.remove (_accessed_widgets.indexOf (to));
        }

        _accessed_widgets.push (to);
        to->function().change_manager().setActive (true);

        foreach (QAction* action, to->actions())
        {
          action->setVisible (true);
        }
        _document_specific_action_menu->menuAction()->setVisible
          (!to->actions().isEmpty());
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

        foreach (document_view* view, findChildren<document_view*>())
        {
          menu->addAction (view->toggleViewAction());
        }

        return menu;
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
        _action_execute_current_file_locally_via_prompt->setEnabled (false);
        _action_execute_current_file_locally_from_file->setEnabled (false);

        runtime_toolbar->addAction
          (_action_execute_current_file_locally_via_prompt);
        runtime_toolbar->addAction
          (_action_execute_current_file_locally_from_file);
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

        QAction* close_action = file_menu->addAction
          (tr ("close_document"), this, SLOT (close_document()));
        QAction* quit_action = file_menu->addAction
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

        struct temporary_path_type
        {
          temporary_path_type()
            : _path (boost::filesystem::unique_path())
          {
            boost::filesystem::create_directories (_path);
          }

          ~temporary_path_type()
          {
            boost::filesystem::remove_all (_path);
          }

          operator boost::filesystem::path() const
          {
            return _path;
          }

          boost::filesystem::path _path;
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

        we::mgmt::type::activity_t prepare_activity
          ( const QStack<document_view*>& accessed_widgets
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

          return xml::parse::xml_to_we (function, state);
        }

        bool put_token ( we::mgmt::type::activity_t& activity
                       , const std::string& port_name
                       , const std::string& value
                       )
        {
          try
          {
            std::size_t k (0);
            std::string::const_iterator begin (value.begin());
            fhg::util::parse::position pos (k, begin, value.end());

            try
            {
              we::util::token::put (activity, port_name, ::value::read (pos));
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

          BOOST_FOREACH ( const we::mgmt::type::activity_t::token_on_port_t& top
                        , activity.output()
                        )
          {
            std::stringstream tmp;
            tmp << "on " << activity.transition().get_port (top.second).name()
                << ": " << top.first;
            QMessageBox msgBox;
            msgBox.setText (QString::fromStdString (tmp.str()));
            msgBox.exec();
          }
        }
      }

      void editor_window::execute_locally_inputs_via_prompt()
      try
      {
        const temporary_path_type temporary_path;

        we::mgmt::type::activity_t activity
          (prepare_activity (_accessed_widgets, temporary_path));

        BOOST_FOREACH ( const std::string& port_name
                      , activity.transition().port_names (we::type::PORT_IN)
                      )
        {
          bool retry (true);
          while (retry)
          {
            bool ok;
            const std::string value
              ( QInputDialog::getText ( this
                                      , tr ("value_for_input_token")
                                      , tr ("enter_value_for_input_port_%1")
                                      .arg (QString::fromStdString (port_name))
                                      , QLineEdit::Normal
                                      , "[]"
                                      , &ok
                                      ).toStdString()
              );
            if (!ok)
            {
              return;
            }

            retry = put_token (activity, port_name, value);
          }
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

      void editor_window::execute_locally_inputs_from_file()
      try
      {
        const temporary_path_type temporary_path;

        we::mgmt::type::activity_t activity
          (prepare_activity (_accessed_widgets, temporary_path));

        bool ok;
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
