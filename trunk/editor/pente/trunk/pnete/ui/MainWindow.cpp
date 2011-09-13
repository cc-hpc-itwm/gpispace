#include "MainWindow.hpp"

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

#include "convenience_splitter.hpp"
#include "GraphView.hpp"
#include "splittable_tab_widget.hpp"
#include "StructureView.hpp"
#include "TransitionLibraryModel.hpp"
#include "view_manager.hpp"

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      MainWindow::MainWindow (const QString& load, QWidget* parent)
      : QMainWindow (parent)
      , _transitionLibrary (NULL)
      , _view_manager (new view_manager (this))
      {
        setCentralWidget (_view_manager->splitter());
        setupMenuAndToolbar();
        setupTransitionLibrary();
        setupStructureView (load);

        connect (_view_manager
                , SIGNAL (view_changed (view_type*))
                , SLOT (view_changed (view_type*))
                );
        connect (_view_manager
                , SIGNAL (scene_changed (scene_type*))
                , SLOT (scene_changed (scene_type*))
                );
      }

      void MainWindow::scene_changed (graph::Scene*)
      {
      }

      void MainWindow::view_changed (ui::GraphView* view)
      {
        view->emit_current_zoom_level();
      }

      void MainWindow::expandTree()
      {
        _transitionLibrary->expandAll();
      }

      void MainWindow::setTransitionLibraryPath (const QString& path)
      {
        TransitionLibraryModel* fsmodel
            (new TransitionLibraryModel(QDir (path), this));
        _transitionLibrary->setModel (fsmodel);
        _transitionLibrary->expandAll();
        _transitionLibrary->setColumnWidth (0, 230);                            // hardcoded constant
        _transitionLibrary->setColumnWidth (1, 20);                             // hardcoded constant

        connect (fsmodel, SIGNAL (layoutChanged()), SLOT (expandTree()));
      }

      void MainWindow::addTransitionLibraryUserPath ( const QString& path
                                                    , bool trusted
                                                    )
      {
        qobject_cast<TransitionLibraryModel*> (_transitionLibrary->model())->
            addContentFromDirectory (path, trusted);
        _transitionLibrary->expandAll();
      }

      void MainWindow::setupMenuAndToolbar()
      {
        setWindowTitle (tr ("SDPA editor"));
        resize (1000, 800);                                                     // hardcoded constant

        //! \todo icons for toolbar.
        QAction* createAction (new QAction (tr ("Create"), this));
        QAction* openAction (new QAction (tr ("Open"), this));
        QAction* saveAction (new QAction (tr ("Save"), this));
        QAction* closeAction (new QAction (tr ("Close"), this));
        QAction* quitAction (new QAction (tr ("Quit"), this));

        createAction->setShortcuts (QKeySequence::New);
        openAction->setShortcuts (QKeySequence::Open);
        saveAction->setShortcuts (QKeySequence::Save);
        closeAction->setShortcuts (QKeySequence::Close);
        quitAction->setShortcuts (QKeySequence::Quit);

        connect (createAction, SIGNAL (triggered()), SLOT (create()));
        connect (openAction, SIGNAL (triggered()), SLOT (open()));
        connect (saveAction, SIGNAL (triggered()), SLOT (save()));
        connect (closeAction, SIGNAL (triggered()), SLOT (close_tab()));
        connect (quitAction, SIGNAL (triggered()), SLOT (quit()));

        QMenuBar* menuBar (new QMenuBar (this));
        menuBar->setSizePolicy
            (QSizePolicy (QSizePolicy::Ignored, QSizePolicy::Preferred));
        QMenu* menuFile (new QMenu (tr ("File"), menuBar));

        menuBar->addAction (menuFile->menuAction());
        menuFile->addAction (createAction);
        menuFile->addAction (openAction);
        menuFile->addAction (saveAction);
        menuFile->addAction (closeAction);
        menuFile->addAction (quitAction);

        QMenu* menuEdit (new QMenu (tr ("Edit"), menuBar));

        menuBar->addAction (menuEdit->menuAction());

        //! \todo This is nearly duplicate code, also available in graph::scene.
        QMenu* menu_new (new QMenu (tr ("new"), this));
        QAction* action_add_transition (menu_new->addAction (tr ("transition")));
        _view_manager->connect ( action_add_transition
                               , SIGNAL (triggered())
                               , SLOT (current_scene_add_transition())
                               );

        QAction* action_add_place (menu_new->addAction (tr ("place")));
        _view_manager->connect ( action_add_place
                               , SIGNAL (triggered())
                               , SLOT (current_scene_add_place())
                               );

        menu_new->addSeparator();

        QAction* action_add_struct (menu_new->addAction (tr ("struct")));
        _view_manager->connect ( action_add_struct
                               , SIGNAL (triggered())
                               , SLOT (current_scene_add_struct())
                               );

        menuEdit->addMenu (menu_new);

        setMenuBar (menuBar);

        QToolBar* mainToolBar (new QToolBar (this));
        addToolBar (Qt::TopToolBarArea, mainToolBar);
        setUnifiedTitleAndToolBarOnMac (true);

        mainToolBar->addAction (createAction);
        mainToolBar->addAction (openAction);
        mainToolBar->addAction (saveAction);

        QWidget* spacer (new QWidget (this));
        spacer->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
        mainToolBar->addWidget (spacer);

        //! \note These constants are not only hardcoded but also duplicate.
        //! \todo Use the same constants as in view_type.
        const int min_zoom_value (30);                                          // hardcoded constant
        const int max_zoom_value (300);                                         // hardcoded constant
        const int default_zoom_value (100);                                     // hardcoded constant
        const int maximum_slider_length (200);                                  // hardcoded constant

        QSlider* zoomSlider (new QSlider(Qt::Horizontal, this));
        zoomSlider->setMaximumSize
            (QSize (maximum_slider_length, maximum_slider_length));
        zoomSlider->setRange (min_zoom_value, max_zoom_value);
        mainToolBar->addWidget (zoomSlider);

        //! \todo Horizontal, it looks a bit weird on my window manager.
        zoomSlider->connect ( mainToolBar
                            , SIGNAL (orientationChanged (Qt::Orientation))
                            , SLOT (setOrientation (Qt::Orientation))
                            );

        //! \todo icon. ._.
        QSpinBox* zoomSpinBox (new QSpinBox (this));
        zoomSpinBox->setSuffix ("%");
        zoomSpinBox->setRange (min_zoom_value, max_zoom_value);
        mainToolBar->addWidget (zoomSpinBox);

        zoomSpinBox->connect ( zoomSlider
                             , SIGNAL (valueChanged (int))
                             , SLOT (setValue (int))
                             );
        zoomSlider->connect ( zoomSpinBox
                            , SIGNAL (valueChanged (int))
                            , SLOT (setValue (int))
                            );
        _view_manager->connect ( zoomSpinBox
                               , SIGNAL (valueChanged (int))
                               , SLOT (zoom_current_view (int))
                               );
        zoomSpinBox->connect ( _view_manager
                             , SIGNAL (zoomed (int))
                             , SLOT (setValue (int))
                             );
        zoomSlider->connect ( _view_manager
                            , SIGNAL (zoomed (int))
                            , SLOT (setValue (int))
                            );

        zoomSlider->setValue (default_zoom_value);
      }

      void MainWindow::setupTransitionLibrary()
      {
        QDockWidget* transitionLibraryDockWidget
            (new QDockWidget (tr ("Library"), this));
        transitionLibraryDockWidget->setMinimumSize (QSize (254, 304));         // hardcoded constant
        transitionLibraryDockWidget->setAllowedAreas
            (Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);

        QWidget* transitionLibraryDockWidgetContents (new QWidget);

        QGridLayout* transitionLibraryDockWidgetLayout
            (new QGridLayout (transitionLibraryDockWidgetContents));
        transitionLibraryDockWidgetLayout->setContentsMargins (2, 2, 2, 2);

        _transitionLibrary = new QTreeView (transitionLibraryDockWidgetContents);
        _transitionLibrary->setFrameShape (QFrame::StyledPanel);
        _transitionLibrary->setFrameShadow (QFrame::Sunken);
        _transitionLibrary->setDragDropMode (QAbstractItemView::DragOnly);
        _transitionLibrary->header()->hide();
        //! \todo Not resizable?
        //_transitionLibrary->header()->setCascadingSectionResizes(true);

        transitionLibraryDockWidgetLayout->addWidget (_transitionLibrary);
        transitionLibraryDockWidget->setWidget
            (transitionLibraryDockWidgetContents);

        addDockWidget (Qt::RightDockWidgetArea, transitionLibraryDockWidget);
      }

      void MainWindow::setupStructureView (const QString& load)
      {
        QDockWidget* dockWidget (new QDockWidget (tr ("Structure"), this));
        dockWidget->setMinimumSize (QSize (254, 304));                          // hardcoded constant
        dockWidget->setAllowedAreas ( Qt::LeftDockWidgetArea
                                    | Qt::RightDockWidgetArea
                                    );

        QWidget* dockWidgetContents (new QWidget());
        QGridLayout* dockWidgetLayout (new QGridLayout (dockWidgetContents));
        dockWidgetLayout->setContentsMargins (2, 2, 2, 2);

        _structureView = new StructureView(load, dockWidgetContents);

        dockWidgetLayout->addWidget (_structureView);
        dockWidget->setWidget (dockWidgetContents);

        addDockWidget (Qt::LeftDockWidgetArea, dockWidget);
      }

      void MainWindow::create()
      {
        _view_manager->create_new_scene_and_view();
      }

      void MainWindow::save()
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

        _view_manager->save_current_scene (filename);
      }

      void MainWindow::open()
      {
        QString filename (QFileDialog::getOpenFileName
            (this, tr ("Load net"), QDir::homePath(), tr ("XML files (*.xml)")));

        if (filename.isEmpty())
        {
          return;
        }

        _structureView->fromFile (filename.toStdString());

        _view_manager->create_view_for_file (filename);
      }

      void MainWindow::close_tab()
      {
        _view_manager->current_tab_widget()->close_current_tab();
      }

      void MainWindow::quit()
      {
        //! \todo Warn, if unsaved documents open.
        close();
      }
    }
  }
}
