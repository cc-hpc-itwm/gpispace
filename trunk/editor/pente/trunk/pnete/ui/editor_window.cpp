#include <pnete/ui/editor_window.hpp>

#include <QAction>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QGridLayout>
#include <QHeaderView>
#include <QMenu>
#include <QMenuBar>
#include <QSpinBox>
#include <QString>
#include <QToolBar>
#include <QTreeView>
#include <QWidget>

#include <pnete/ui/GraphScene.hpp>
#include <pnete/ui/GraphView.hpp>
#include <pnete/ui/StructureView.hpp>
#include <pnete/ui/TransitionLibraryModel.hpp>
#include <pnete/ui/view_manager.hpp>

#include <pnete/data/manager.hpp>

#include <pnete/traverse/display.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      const Qt::DockWidgetArea dock_position (Qt::LeftDockWidgetArea);

      //! \note These constants are not only hardcoded but also duplicate.
      //! \todo Use the same constants as in GraphView.
      static const int min_zoom_value (30);                                     // hardcoded constant
      static const int max_zoom_value (300);                                    // hardcoded constant
      static const int default_zoom_value (100);                                // hardcoded constant
      static const int maximum_slider_length (200);                             // hardcoded constant

      editor_window::editor_window (QWidget* parent)
        : QMainWindow (parent)
        , _transition_library (NULL)
        , _view_manager (new view_manager (this))
      {
        setup();
      }

      void editor_window::setup()
      {
        setWindowTitle (tr ("editor_window_title"));

        setup_structure_view();
        setup_initial_document ();
        setup_transition_library();
        setup_menu_and_toolbar();

        setDocumentMode (true);
        setDockNestingEnabled (true);
        setTabPosition (Qt::AllDockWidgetAreas, QTabWidget::North);

        connect (_view_manager
                , SIGNAL (view_changed (GraphView*))
                , SLOT (view_changed (GraphView*))
                );
        connect (_view_manager
                , SIGNAL (scene_changed (graph::Scene*))
                , SLOT (scene_changed (graph::Scene*))
                );
      }

      void editor_window::setup_initial_document ()
      {
        //! \note this is a dummy only.
        setCentralWidget (new QWidget());
        centralWidget()->hide();
        create();
      }

      void editor_window::scene_changed (graph::Scene*)
      {
      }

      void editor_window::view_changed (GraphView* view)
      {
        view->emit_current_zoom_level();
      }

      void editor_window::expand_library()
      {
        _transition_library->expandAll();
      }

      void editor_window::set_transition_library_path (const QString& path)
      {
        TransitionLibraryModel* model
            (new TransitionLibraryModel (QDir (path), this));
        _transition_library->setModel (model);
        _transition_library->expandAll();
        _transition_library->setColumnWidth (0, 230);                           // hardcoded constant
        _transition_library->setColumnWidth (1, 20);                            // hardcoded constant

        connect (model, SIGNAL (layoutChanged()), SLOT (expand_library()));
      }

      void editor_window::add_transition_library_user_path ( const QString& path
                                                           , bool trusted
                                                           )
      {
        qobject_cast<TransitionLibraryModel*> (_transition_library->model())->
            addContentFromDirectory (path, trusted);
        expand_library();
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

        QAction* create_action (new QAction (tr ("new_net"), this));
        QAction* open_action (new QAction (tr ("open_net"), this));
        QAction* save_action (new QAction (tr ("save_net"), this));
        QAction* close_action (new QAction (tr ("close_current_window"), this));
        QAction* quit_action (new QAction (tr ("quit_application"), this));

        create_action->setShortcuts (QKeySequence::New);
        open_action->setShortcuts (QKeySequence::Open);
        save_action->setShortcuts (QKeySequence::Save);
        close_action->setShortcuts (QKeySequence::Close);
        quit_action->setShortcuts (QKeySequence::Quit);

        connect (create_action, SIGNAL (triggered()), SLOT (create()));
        connect (open_action, SIGNAL (triggered()), SLOT (open()));
        connect (save_action, SIGNAL (triggered()), SLOT (save()));
        connect (close_action, SIGNAL (triggered()), SLOT (close_document()));
        connect (quit_action, SIGNAL (triggered()), SLOT (quit()));

        file_menu->addAction (create_action);
        file_menu->addAction (open_action);
        file_menu->addAction (save_action);
        file_menu->addAction (close_action);
        file_menu->addAction (quit_action);

        file_tool_bar->addAction (create_action);
        file_tool_bar->addAction (open_action);
        file_tool_bar->addAction (save_action);
      }

      void editor_window::setup_edit_actions (QMenuBar* menu_bar)
      {
        QMenu* edit_menu (new QMenu (tr ("edit_menu"), menu_bar));
        menu_bar->addAction (edit_menu->menuAction());

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
        zoom_slider->setMaximumWidth (maximum_slider_length);
        zoom_slider->setRange (min_zoom_value, max_zoom_value);
        zoom_tool_bar->addWidget (zoom_slider);

        zoom_slider->setValue (default_zoom_value);

        QSpinBox* zoom_spin_box (new QSpinBox (this));
        zoom_spin_box->setSuffix ("%");
        zoom_spin_box->setRange (min_zoom_value, max_zoom_value);
        zoom_tool_bar->addWidget (zoom_spin_box);

        zoom_spin_box->setValue (default_zoom_value);

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
        //_view_manager->current_view_zoom (default_zoom_value);
      }

      QMenu* editor_window::createPopupMenu()
      {
        QMenu* menu (QMainWindow::createPopupMenu());

        QAction* duplicate_view_action (new QAction (tr ("duplicate_view"), menu));
        QAction* close_action (new QAction (tr ("close_current_window"), menu));

        _view_manager->connect ( duplicate_view_action
                               , SIGNAL (triggered())
                               , SLOT (duplicate_active_widget())
                               );
        _view_manager->connect ( close_action
                               , SIGNAL (triggered())
                               , SLOT (current_widget_close())
                               );

        //! \todo Add them BEFORE, not after the existing actions.
        menu->addSeparator();
        menu->addAction (duplicate_view_action);
        menu->addAction (close_action);

        return menu;
      }

      void editor_window::setup_window_actions (QMenuBar* menu_bar)
      {
        QMenu* windows_menu (new QMenu (tr ("windos_menu"), menu_bar));
        menu_bar->addAction (windows_menu->menuAction());

        QAction* duplicate_current_action
          (new QAction (tr ("duplicate_current_window"), this));

        duplicate_current_action->setShortcuts (QKeySequence::AddTab);

        windows_menu->addAction (duplicate_current_action);

        _view_manager->connect ( duplicate_current_action
                               , SIGNAL (triggered())
                               , SLOT (duplicate_active_widget())
                               );

        //menu_bar->addAction(createPopupMenu()->menuAction());
        //! \todo Open createPopupMenu() on menu_action::triggered().
        //! \todo Actually differ between documents and library / structure there.
      }

      class dock_widget : public QDockWidget
      {
        public:
          dock_widget (const QString& n, QWidget* child, QWidget* parent = NULL)
          : QDockWidget (n, parent)
          {
            setAllowedAreas (dock_position);
            setFeatures ( QDockWidget::DockWidgetClosable
                        | QDockWidget::DockWidgetMovable
                        );

            setWidget (child);
          }
      };

      void editor_window::setup_transition_library()
      {
        _transition_library = new QTreeView (this);
        _transition_library->setDragDropMode (QAbstractItemView::DragOnly);
        _transition_library->header()->hide();

        addDockWidget ( dock_position
                      , new dock_widget ( tr ("library_window")
                                        , _transition_library
                                        , this
                                        )
                      , Qt::Horizontal
                      );
      }

      void editor_window::setup_structure_view ()
      {
        _structure_view = new StructureView (this);

        addDockWidget ( dock_position
                      , new dock_widget ( tr ("structure_window")
                                        , _structure_view
                                        , this
                                        )
                      , Qt::Horizontal
                      );
      }

      void editor_window::create()
      {
        create_windows (data::manager::instance().create());
      }

      void editor_window::save()
      {
        QString filename (QFileDialog::getSaveFileName
           (this, tr ("Save net"), QDir::homePath(), tr ("XML files (*.xml)")));

        if (filename.isEmpty())
        {
          return;
        }

        if (!filename.endsWith (".xml"))
        {
          filename.append (".xml");
        }

        save (filename);
      }

      void editor_window::open()
      {
        QString filename (QFileDialog::getOpenFileName
           (this, tr ("Load net"), QDir::homePath(), tr ("XML files (*.xml)")));

        if (filename.isEmpty())
        {
          return;
        }

        open (filename);
      }

      void editor_window::create_windows (data::internal::ptr data)
      {
        _view_manager->create_widget
          (*weaver::display::weaver(data->function()).proxy());
        _structure_view->append (data);
      }

      void editor_window::save (const QString& filename)
      {
        //        _view_manager->_scene (filename);
      }
      void editor_window::open (const QString& filename)
      {
        create_windows (data::manager::instance().load (filename));
      }

      void editor_window::close_document()
      {
        _view_manager->current_widget_close();
      }

      void editor_window::quit()
      {
        //! \todo Warn, if unsaved documents open.
        close();
      }
    }
  }
}
