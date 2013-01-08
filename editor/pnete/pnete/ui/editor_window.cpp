// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/editor_window.hpp>

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QFileDialog>
#include <QGridLayout>
#include <QMenu>
#include <QMenuBar>
#include <QSettings>
#include <QSpinBox>
#include <QString>
#include <QToolBar>
#include <QTreeView>
#include <QUndoGroup>
#include <QUndoView>
#include <QWidget>

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
        , _action_save_current_file (new QAction (tr ("save"), this))
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

        add_on_top_of_current_widget (document_view_factory (proxy));

        _action_save_current_file->setEnabled (true);
      }

      void editor_window::create_windows (data::internal_type* data)
      {
        create_widget (data->root_proxy());
        _structure_view->append (data);
      }

      void editor_window::add_on_top_of_current_widget (document_view* w)
      {
        if (!_accessed_widgets.empty())
        {
          tabifyDockWidget (_accessed_widgets.top(), w);
        }
        else
        {
          addDockWidget (Qt::LeftDockWidgetArea, w, Qt::Horizontal);
        }

        w->show();
        w->raise();
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
          document_view* current (_accessed_widgets.top());
          _accessed_widgets.pop ();
          removeDockWidget (current);
          delete current;

          if (!_accessed_widgets.empty())
          {
            _accessed_widgets.top()->widget()->setFocus();
          }
          else
          {
            _action_save_current_file->setEnabled (false);
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
      }

      void editor_window::setup_file_actions (QMenuBar* menu_bar)
      {
        QMenu* file_menu (new QMenu (tr ("file_menu"), menu_bar));
        menu_bar->addAction (file_menu->menuAction());

        QToolBar* file_tool_bar (new QToolBar (tr ("file_tool_bar"), this));
        addToolBar (Qt::TopToolBarArea, file_tool_bar);
        file_tool_bar->setFloatable (false);

        QMenu* menu_new (new QMenu (tr("new")));
        QAction* action_new_expression (menu_new->addAction (tr("expression")));
        action_new_expression->setShortcut (QKeySequence("Ctrl+E"));
        connect ( action_new_expression
                , SIGNAL (triggered())
                , SLOT (slot_new_expression())
                );
        QAction* action_new_module_call (menu_new->addAction (tr("module_call")));
        action_new_module_call->setShortcut (QKeySequence("Ctrl+M"));
        connect ( action_new_module_call
                , SIGNAL (triggered())
                , SLOT (slot_new_module_call())
                );
        QAction* action_new_net (menu_new->addAction (tr("net")));
        connect ( action_new_net
                , SIGNAL (triggered())
                , SLOT (slot_new_net())
                );
        action_new_net->setShortcut (QKeySequence::New);
        file_menu->addMenu (menu_new);

        QAction* open_action (new QAction (tr ("open"), this));

        QAction* close_action (new QAction (tr ("close_current_window"), this));
        QAction* quit_action (new QAction (tr ("quit_application"), this));

        open_action->setShortcut (QKeySequence::Open);
        _action_save_current_file->setShortcuts (QKeySequence::Save);
        close_action->setShortcut (QKeySequence::Close);
        quit_action->setShortcut (QKeySequence::Quit);

        _action_save_current_file->setEnabled (false);

        connect (open_action, SIGNAL (triggered()), SLOT (open()));
        connect ( _action_save_current_file
                , SIGNAL (triggered())
                , SLOT (save_file())
                );
        connect (close_action, SIGNAL (triggered()), SLOT (close_document()));
        connect (quit_action, SIGNAL (triggered()), SLOT (quit()));

        file_menu->addAction (open_action);
        file_menu->addAction (_action_save_current_file);
        file_menu->addAction (close_action);
        file_menu->addAction (quit_action);

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
