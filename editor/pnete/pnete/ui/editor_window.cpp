// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/editor_window.hpp>

#include <QAction>
#include <QDir>
#include <QFileDialog>
#include <QGridLayout>
#include <QMenu>
#include <QMenuBar>
#include <QSpinBox>
#include <QString>
#include <QToolBar>
#include <QTreeView>
#include <QWidget>
#include <QCloseEvent>
#include <QSettings>
#include <QUndoView>

#include <pnete/ui/StructureView.hpp>
#include <pnete/ui/TransitionLibraryModel.hpp>
#include <pnete/ui/dock_widget.hpp>
#include <pnete/ui/document_view.hpp>
#include <pnete/ui/graph/scene.hpp>
#include <pnete/ui/size.hpp>
#include <pnete/ui/transition_library_view.hpp>
#include <pnete/ui/view_manager.hpp>

#include <pnete/data/manager.hpp>
#include <pnete/data/internal.hpp>

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
        , _view_manager (new view_manager (this))
        , _structure_view (new StructureView (this))
        , _structure_view_dock
          (new dock_widget (tr ("structure_window"), _structure_view))
        , _undo_view_dock ( new dock_widget
                            ( tr ("undo_window")
                            , _view_manager->create_undo_view (this)
                            )
                          )
        , _windows_menu (NULL)
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

        readSettings();
      }

      void editor_window::add_transition_library_path ( const QString& path
                                                      , bool trusted
                                                      )
      {
        qobject_cast<TransitionLibraryModel*> (_transition_library->model())->
            addContentFromDirectory (path, trusted);
      }

      void editor_window::setup_menu_and_toolbar()
      {
        QMenuBar* menu_bar (new QMenuBar (this));
        setMenuBar (menu_bar);

        setUnifiedTitleAndToolBarOnMac (true);

        //! \todo icons for toolbar.
        setup_file_actions (menu_bar);
        setup_edit_actions (menu_bar);
        setup_zoom_actions (menu_bar);
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
        QAction* save_action (_view_manager->action_save_current_file());
        QAction* close_action (new QAction (tr ("close_current_window"), this));
        QAction* quit_action (new QAction (tr ("quit_application"), this));

        open_action->setShortcut (QKeySequence::Open);
        close_action->setShortcut (QKeySequence::Close);
        quit_action->setShortcut (QKeySequence::Quit);

        connect (open_action, SIGNAL (triggered()), SLOT (open()));
        connect (close_action, SIGNAL (triggered()), SLOT (close_document()));
        connect (quit_action, SIGNAL (triggered()), SLOT (quit()));

        file_menu->addAction (open_action);
        file_menu->addAction (save_action);
        file_menu->addAction (close_action);
        file_menu->addAction (quit_action);

        file_tool_bar->addAction (action_new_expression);
        file_tool_bar->addAction (action_new_module_call);
        file_tool_bar->addAction (action_new_net);
        file_tool_bar->addAction (open_action);
        file_tool_bar->addAction (save_action);
      }

      void editor_window::setup_edit_actions (QMenuBar* menu_bar)
      {
        QMenu* edit_menu (new QMenu (tr ("edit_menu"), menu_bar));
        menu_bar->addAction (edit_menu->menuAction());

        QAction* undo_action
          ( _view_manager->undo_group()->createUndoAction
            (this, tr ("undo_prefix"))
          );
        QAction* redo_action
          ( _view_manager->undo_group()->createRedoAction
            (this, tr ("redo_prefix"))
          );
        undo_action->setShortcuts (QKeySequence::Undo);
        redo_action->setShortcuts (QKeySequence::Redo);
        edit_menu->addAction (undo_action);
        edit_menu->addAction (redo_action);
        edit_menu->addSeparator();

        QAction* auto_layout_action (new QAction (tr ("auto_layout"), this));
        QAction* add_transition_action (new QAction (tr ("add_transition"), this));
        QAction* add_place_action (new QAction (tr ("add_place"), this));
        QAction* add_struct_action (new QAction (tr ("add_struct"), this));

        _view_manager->connect ( auto_layout_action
                               , SIGNAL (triggered())
                               , SLOT (current_scene_auto_layout())
                               );
        _view_manager->connect ( add_transition_action
                               , SIGNAL (triggered())
                               , SLOT (current_scene_add_transition())
                               );
        _view_manager->connect ( add_place_action
                               , SIGNAL (triggered())
                               , SLOT (current_scene_add_place())
                               );
        _view_manager->connect ( add_struct_action
                               , SIGNAL (triggered())
                               , SLOT (current_scene_add_struct())
                               );

        edit_menu->addAction (auto_layout_action);
        edit_menu->addSeparator();
        edit_menu->addAction (add_transition_action);
        edit_menu->addAction (add_place_action);
        edit_menu->addSeparator();
        edit_menu->addAction (add_struct_action);
      }

      void editor_window::setup_zoom_actions (QMenuBar* menu_bar)
      {
        QMenu* zoom_menu (new QMenu (tr ("zoom_menu"), menu_bar));
        menu_bar->addAction (zoom_menu->menuAction());

        QToolBar* zoom_tool_bar (new QToolBar (tr ("zoom_tool_bar"), this));
        addToolBar (Qt::TopToolBarArea, zoom_tool_bar);
        zoom_tool_bar->setAllowedAreas ( Qt::TopToolBarArea
                                       | Qt::BottomToolBarArea
                                       );
        zoom_tool_bar->setFloatable (false);

        QAction* zoom_in_action (new QAction (tr ("zoom_in"), this));
        QAction* zoom_out_action (new QAction (tr ("zoom_out"), this));
        QAction* zoom_default_action (new QAction (tr ("zoom_default"), this));

        zoom_in_action->setShortcuts (QKeySequence::ZoomIn);
        zoom_out_action->setShortcuts (QKeySequence::ZoomOut);
        zoom_default_action->setShortcut (QKeySequence ("Ctrl+*"));

        _view_manager->connect ( zoom_in_action
                               , SIGNAL (triggered())
                               , SLOT (current_view_zoom_in())
                               );
        _view_manager->connect ( zoom_out_action
                               , SIGNAL (triggered())
                               , SLOT (current_view_zoom_out())
                               );
        _view_manager->connect ( zoom_default_action
                               , SIGNAL (triggered())
                               , SLOT (current_view_reset_zoom())
                               );

        zoom_menu->addAction (zoom_in_action);
        zoom_menu->addAction (zoom_out_action);
        zoom_menu->addAction (zoom_default_action);

        zoom_tool_bar->addAction (zoom_in_action);
        zoom_tool_bar->addAction (zoom_out_action);
        zoom_tool_bar->addAction (zoom_default_action);

        QSlider* zoom_slider (new QSlider (Qt::Horizontal, this));
        zoom_slider->setMaximumWidth (size::zoom::slider::max_length());
        zoom_slider->setRange ( size::zoom::min_value()
                              , size::zoom::max_value()
                              );
        zoom_tool_bar->addWidget (zoom_slider);

        zoom_slider->setValue (size::zoom::default_value());

        QSpinBox* zoom_spin_box (new QSpinBox (this));
        zoom_spin_box->setSuffix ("%");
        zoom_spin_box->setRange ( size::zoom::min_value()
                                , size::zoom::max_value()
                                );
        zoom_tool_bar->addWidget (zoom_spin_box);

        zoom_spin_box->setValue (size::zoom::default_value());

        zoom_slider->connect ( zoom_spin_box
                              , SIGNAL (valueChanged (int))
                              , SLOT (setValue (int))
                              );
        _view_manager->connect ( zoom_spin_box
                               , SIGNAL (valueChanged (int))
                               , SLOT (current_view_zoom (int))
                               );

        _view_manager->connect ( zoom_slider
                               , SIGNAL (valueChanged (int))
                               , SLOT (current_view_zoom (int))
                               );
        zoom_spin_box->connect ( zoom_slider
                               , SIGNAL (valueChanged (int))
                               , SLOT (setValue (int))
                               );

        zoom_spin_box->connect ( _view_manager
                               , SIGNAL (zoomed (int))
                               , SLOT (setValue (int))
                               );
        zoom_slider->connect ( _view_manager
                              , SIGNAL (zoomed (int))
                              , SLOT (setValue (int))
                              );

        //! \todo This be segfault without view.
        //_view_manager->current_view_zoom (size::zoom::default_value());
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
                        , _view_manager
                        , SLOT (duplicate_active_widget())
                        , QKeySequence::AddTab
                        );
        menu->addAction ( tr ("close_current_window")
                        , _view_manager
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

      void editor_window::setup_window_actions (QMenuBar* menu_bar)
      {
        _windows_menu = menu_bar->addMenu (tr ("windows_menu"));

        connect ( _windows_menu
                , SIGNAL (aboutToShow())
                , SLOT (update_window_menu())
                );
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

        open (filename);
      }

      void editor_window::create_windows (data::internal_type* data)
      {
        _view_manager->create_widget (data->root_proxy());
        _structure_view->append (data);
      }

      void editor_window::open (const QString& filename)
      {
        create_windows (data::manager::instance().load (filename));
      }

      void editor_window::close_document()
      {
        //! \todo Should close all windows of that document!
        _view_manager->current_widget_close();
      }

      void editor_window::quit()
      {
        //! \todo Warn, if unsaved documents open.
        close();
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
