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
#include <QFileDialog>
#include <QDir>
#include <QSlider>
#include <QSpinBox>
#include <QTextStream>

#include "GraphView.hpp"
#include "TransitionLibraryModel.hpp"
#include "GraphScene.hpp"
#include "helper/GraphTraverser.hpp"
#include "helper/TraverserReceiver.hpp"

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      MainWindow::MainWindow(QWidget* parent)
      : QMainWindow(parent)
      , _transitionLibrary(NULL)
      , _graphicsView(NULL)
      , _scene(NULL)
      {
        setupCentralWidget();
        setupMenuAndToolbar();
        setupTransitionLibrary();
      }

      void MainWindow::expandTree()
      {
        _transitionLibrary->expandAll();
      }

      void MainWindow::setTransitionLibraryPath(const QString& path)
      {
        TransitionLibraryModel* fsmodel = new TransitionLibraryModel(QDir(path), this);
        _transitionLibrary->setModel(fsmodel);
        _transitionLibrary->expandAll();
        _transitionLibrary->setColumnWidth(0, 230);                             // hardcoded constant
        _transitionLibrary->setColumnWidth(1, 20);                              // hardcoded constant

        connect(fsmodel, SIGNAL(layoutChanged()), SLOT(expandTree()));
      }

      void MainWindow::addTransitionLibraryUserPath(const QString& path, bool trusted)
      {
        qobject_cast<TransitionLibraryModel*>(_transitionLibrary->model())->addContentFromDirectory(path, trusted);
        _transitionLibrary->expandAll();
      }

      graph::Scene * MainWindow::scene ()
      {
        return _scene;
      }

      void MainWindow::setupMenuAndToolbar()
      {
        setWindowTitle(tr("SDPA editor"));
        resize(600+250, 600+20);                                                // hardcoded constant

        //! \todo icons for toolbar.
        QAction* saveAction = new QAction(tr("Save"), this);
        saveAction->setShortcuts(QKeySequence::Save);
        QAction* closeAction = new QAction(tr("Close"), this);
        closeAction->setShortcuts(QKeySequence::Close);

        connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));
        connect(closeAction, SIGNAL(triggered()), this, SLOT(close()));

        //! \todo Do not make this a child of MainWindow on OSX for being global across windows.
        QMenuBar* menuBar = new QMenuBar(this);
        //! \todo This might be wrong on !OSX.
        menuBar->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred));

        QMenu* menuFile = new QMenu(tr("File"), menuBar);

        menuBar->addAction(menuFile->menuAction());
        menuFile->addAction(saveAction);
        menuFile->addAction(closeAction);

        QMenu* menuEdit = new QMenu(tr("Edit"), menuBar);

        menuBar->addAction(menuEdit->menuAction());
        menuEdit->addMenu(scene()->menu_new());

        setMenuBar(menuBar);

        QToolBar* mainToolBar = new QToolBar(this);
        addToolBar(Qt::TopToolBarArea, mainToolBar);
        setUnifiedTitleAndToolBarOnMac(true);

        mainToolBar->addAction(saveAction);

        QWidget* spacer = new QWidget(this);
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mainToolBar->addWidget(spacer);

        //! \todo on !OSX, orientation of the toolbar can change. take care of slider!
        //! \note mainToolBar emits orientationChanged -> set slider orientation.
        QSlider* zoomSlider = new QSlider(Qt::Horizontal, this);
        zoomSlider->setMaximumSize(QSize(200, zoomSlider->height()));           // hardcoded constant
        zoomSlider->setRange(10, 300);                                          // hardcoded constant
        mainToolBar->addWidget(zoomSlider);

        //! \todo icon. ._.
        QSpinBox* zoomSpinBox = new QSpinBox(this);
        zoomSpinBox->setSuffix("%");
        zoomSpinBox->setRange(10, 300);                                         // hardcoded constant
        mainToolBar->addWidget(zoomSpinBox);

        connect(zoomSlider, SIGNAL(valueChanged(int)), zoomSpinBox, SLOT(setValue(int)));
        connect(zoomSpinBox, SIGNAL(valueChanged(int)), zoomSlider, SLOT(setValue(int)));
        connect(zoomSpinBox, SIGNAL(valueChanged(int)), _graphicsView, SLOT(zoom(int)));
        connect(_graphicsView, SIGNAL(zoomed(int)), zoomSpinBox, SLOT(setValue(int)));
        connect(_graphicsView, SIGNAL(zoomed(int)), zoomSlider, SLOT(setValue(int)));

        zoomSlider->setValue(100);                                              // hardcoded constant
      }

      void MainWindow::setupCentralWidget()
      {
        QWidget* centralWidget = new QWidget(this);

        QGridLayout* centralLayout = new QGridLayout(centralWidget);
        centralLayout->setContentsMargins(2, 2, 2, 2);                          // hardcoded constant

        _scene = new graph::Scene(QRectF(-2000.0, -2000.0, 4000.0, 4000.0), this);  // hardcoded constant
        _graphicsView = new GraphView(_scene, centralWidget);

        centralLayout->addWidget(_graphicsView);

        setCentralWidget(centralWidget);
      }

      void MainWindow::setupTransitionLibrary()
      {
        QDockWidget* transitionLibraryDockWidget = new QDockWidget(tr("Library"), this);
        transitionLibraryDockWidget->setMinimumSize(QSize(254, 304));           // hardcoded constant
        transitionLibraryDockWidget->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);

        QWidget* transitionLibraryDockWidgetContents = new QWidget();

        QGridLayout* transitionLibraryDockWidgetLayout = new QGridLayout(transitionLibraryDockWidgetContents);
        transitionLibraryDockWidgetLayout->setContentsMargins(2, 2, 2, 2);

        _transitionLibrary = new QTreeView(transitionLibraryDockWidgetContents);
        _transitionLibrary->setFrameShape(QFrame::StyledPanel);
        _transitionLibrary->setFrameShadow(QFrame::Sunken);
        _transitionLibrary->setDragDropMode(QAbstractItemView::DragOnly);
        _transitionLibrary->header()->setVisible(false);
        //! \todo Not resizable?
        //_transitionLibrary->header()->setCascadingSectionResizes(true);

        transitionLibraryDockWidgetLayout->addWidget(_transitionLibrary);
        transitionLibraryDockWidget->setWidget(transitionLibraryDockWidgetContents);

        addDockWidget(Qt::RightDockWidgetArea, transitionLibraryDockWidget);
      }

      void MainWindow::save()
      {
        if(_scene)
        {
          helper::GraphTraverser traverser(_scene);
          helper::TraverserReceiver receiver;

          QFile xmlFile(QFileDialog::getSaveFileName(this, tr("Save net"), QDir::homePath(), tr("XML files (*.xml)")));

          if (!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))
            return;

          QTextStream out(&xmlFile);
          out << traverser.traverse(&receiver, QFileInfo(xmlFile).baseName()) << "\n";
          out.flush();

          xmlFile.close();
        }
      }
    }
  }
}
