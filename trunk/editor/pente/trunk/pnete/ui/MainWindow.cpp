#include "MainWindow.hpp"

#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QDockWidget>
#include <QHeaderView>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QTreeView>
#include <QGridLayout>
#include <QWidget>
#include <QDir>
#include <QSlider>

#include "GraphView.hpp"
#include "TransitionLibraryModel.hpp"
#include "graph/Scene.hpp"

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      MainWindow::MainWindow(QWidget* parent)
      : QMainWindow(parent)
      {
        setupUi();
        setupCentralWidget();
        setupTransitionLibrary();
      }
      
      void MainWindow::setTransitionLibraryPath(const QString& path)
      {
        TransitionLibraryModel* fsmodel = new TransitionLibraryModel(QDir(path), this);
        transitionLibrary->setModel(fsmodel);
        transitionLibrary->expandAll();
      }

      void MainWindow::addTransitionLibraryUserPath(const QString& path)
      {
        qobject_cast<TransitionLibraryModel*>(transitionLibrary->model())->addContentFromDirectory(path);
        transitionLibrary->expandAll();
      }
      
      void MainWindow::setupUi()
      {
        setWindowTitle(tr("SDPA editor"));
        resize(800, 600);
       
        //! \todo icons for toolbar.
        QAction* saveDummyAction = new QAction(tr("Save"), this);
        saveDummyAction->setShortcuts(QKeySequence::Save);
        QAction* closeDummyAction = new QAction(tr("Close"), this);
        closeDummyAction->setShortcuts(QKeySequence::Close);
        
        connect(closeDummyAction, SIGNAL(triggered()), this, SLOT(close()));
        
        //! \todo Do not make this a child of MainWindow on OSX for being global across windows.
        QMenuBar* menuBar = new QMenuBar(this);
        //! \todo This might be wrong on !OSX.
        menuBar->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred));
        
        QMenu* menuFile = new QMenu(tr("File"), menuBar);
        
        menuBar->addAction(menuFile->menuAction());
        menuFile->addAction(saveDummyAction);
        menuFile->addAction(closeDummyAction);
        
        setMenuBar(menuBar);
        
        QToolBar* mainToolBar = new QToolBar(this);
        addToolBar(Qt::TopToolBarArea, mainToolBar);
        setUnifiedTitleAndToolBarOnMac(true);
        
        mainToolBar->addAction(saveDummyAction);
        
        QWidget* spacer = new QWidget(this);
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mainToolBar->addWidget(spacer);
        //! \todo icon. ._.
        mainToolBar->addAction("Zoom");
        
        //! \todo on !OSX, orientation of the toolbar can change. take care of slider!
        QSlider* zoomSlider = new QSlider(Qt::Horizontal, this);
        zoomSlider->setMaximumSize(QSize(200, zoomSlider->height()));
        mainToolBar->addWidget(zoomSlider);
      }
      
      void MainWindow::setupCentralWidget()
      {
        QWidget* centralWidget = new QWidget(this);
        
        QGridLayout* centralLayout = new QGridLayout(centralWidget);
        centralLayout->setContentsMargins(2, 2, 2, 2);
        
        GraphView* graphicsView = new GraphView(new graph::Scene(this), centralWidget);
        
        centralLayout->addWidget(graphicsView);
        
        setCentralWidget(centralWidget);
      }
        
      void MainWindow::setupTransitionLibrary()
      {
        QDockWidget* transitionLibraryDockWidget = new QDockWidget(tr("Library"), this);
        transitionLibraryDockWidget->setMinimumSize(QSize(263, 228));
        transitionLibraryDockWidget->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
        
        QWidget* transitionLibraryDockWidgetContents = new QWidget();
        
        QGridLayout* transitionLibraryDockWidgetLayout = new QGridLayout(transitionLibraryDockWidgetContents);
        transitionLibraryDockWidgetLayout->setContentsMargins(2, 2, 2, 2);
        
        transitionLibrary = new QTreeView(transitionLibraryDockWidgetContents);
        transitionLibrary->setFrameShape(QFrame::StyledPanel);
        transitionLibrary->setFrameShadow(QFrame::Sunken);
        transitionLibrary->setDragDropMode(QAbstractItemView::DragOnly);
        transitionLibrary->header()->setVisible(true);
        transitionLibrary->header()->setCascadingSectionResizes(true);
        transitionLibrary->header()->setHighlightSections(true);
        transitionLibrary->header()->setProperty("showSortIndicator", QVariant(true));
        
        transitionLibraryDockWidgetLayout->addWidget(transitionLibrary);
        transitionLibraryDockWidget->setWidget(transitionLibraryDockWidgetContents);
        
        addDockWidget(Qt::RightDockWidgetArea, transitionLibraryDockWidget);
      }
    }
  }
}
