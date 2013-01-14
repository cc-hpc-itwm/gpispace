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
        , _action_execute_current_file_locally (NULL)
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
          (data::proxy::root (_accessed_widgets.top()->proxy()), filename);
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
          type& _proxy;

        public:
          document_view_for_proxy (type& proxy)
            : _proxy (proxy)
          { }

          document_view* operator() (expression_proxy& proxy) const
          {
            return new document_view
              ( function (_proxy)
              , _proxy
              , QObject::tr ("<<anonymous expression>>")
              , new expression_widget
                ( data::handle::expression ( proxy.data()
                                           , root (_proxy)->change_manager()
                                           )
                , function (_proxy)
                )
              );
          }

          document_view* operator() (mod_proxy& proxy) const
          {
            return new document_view
              ( function (_proxy)
              , _proxy
              , QObject::tr ("<<anonymous module call>>")
              , new module_call_widget (proxy.data(), function (_proxy))
              );
          }

          document_view* operator() (net_proxy& proxy) const
          {
            return new document_view
              ( function (_proxy)
              , _proxy
              , QObject::tr ("<<anonymous net>>")
              , new graph_view (proxy.display())
              );
          }
        };

        document_view* document_view_factory (type& proxy)
        {
          return boost::apply_visitor (document_view_for_proxy (proxy), proxy);
        }
      }

      void editor_window::create_widget (data::proxy::type& proxy)
      {
        _undo_group->addStack (&data::proxy::root (proxy)->change_manager());

        document_view* doc_view (document_view_factory (proxy));
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
        _action_execute_current_file_locally->setEnabled (true);
      }

      void editor_window::create_windows (data::internal_type* data)
      {
        create_widget (data->root_proxy());
        _structure_view->append (data);
      }

      void editor_window::duplicate_active_widget()
      {
        if (!_accessed_widgets.empty())
        {
          create_widget (_accessed_widgets.top()->proxy());
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
            _action_execute_current_file_locally->setEnabled (false);
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

        _action_execute_current_file_locally = runtime_menu->addAction
          (tr ("execute_locally"), this, SLOT (execute_locally()));
        _action_execute_current_file_locally->setEnabled (false);

        runtime_toolbar->addAction (_action_execute_current_file_locally);
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

      struct output_port_and_token : std::iterator< std::output_iterator_tag
                                                  , output_port_and_token
                                                  >
      {
        output_port_and_token const & operator *() const { return *this; }
        output_port_and_token const & operator++() const { return *this; }
        output_port_and_token const & operator=
          (const we::mgmt::type::activity_t::token_on_port_t & subject) const
        {
          std::stringstream tmp;
          tmp << "on " << subject.second << ": " << subject.first;
          QMessageBox msgBox;
          msgBox.setText (QString::fromStdString (tmp.str()));
          msgBox.exec();
          return *this;
        }
      };

      struct delete_directory_on_scope_exit
      {
        delete_directory_on_scope_exit (const boost::filesystem::path& path)
          : _path (path)
        { }

        ~delete_directory_on_scope_exit()
        {
          boost::filesystem::remove_all (_path);
        }

        boost::filesystem::path _path;
      };

      void editor_window::execute_locally()
      try
      {
        if (_accessed_widgets.empty())
        {
          return;
        }

        const boost::filesystem::path temporary_path
          (boost::filesystem::unique_path());
        boost::filesystem::create_directories (temporary_path);

        const delete_directory_on_scope_exit cleanup (temporary_path);

        xml::parse::state::type state;
        state.path_to_cpp() = temporary_path.string();
        //! \todo Add include and link paths

        const xml::parse::id::ref::function function
          ( data::proxy::function (_accessed_widgets.top()->proxy())
          .get().clone()
          );
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

        we::mgmt::type::activity_t activity
          (xml::parse::xml_to_we (function, state));

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

            std::size_t k (0);
            std::string::const_iterator begin (value.begin());
            fhg::util::parse::position pos (k, begin, value.end());

            try
            {
              try
              {
                we::util::token::put (activity, port_name, ::value::read (pos));
                retry = false;
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
              retry = true;
            }
          }
        }

        activity.inject_input();

        we::loader::loader loader;
        loader.append_search_path (temporary_path / "pnetc" / "op");

        eval_context context (loader);

        //! \todo Somehow redirect stdout to buffer
        activity.execute (&context);
        activity.collect_output();

        std::copy ( activity.output().begin()
                  , activity.output().end()
                  , output_port_and_token()
                  );
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
