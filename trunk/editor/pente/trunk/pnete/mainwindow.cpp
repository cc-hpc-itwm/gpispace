#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPushButton>
#include <QFileSystemModel>

#include "graph/Scene.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->graphicsView->setScene(new fhg::pnete::graph::Scene(ui->graphicsView));

    QFileSystemModel* fsmodel = new QFileSystemModel(this);
    fsmodel->setRootPath( "/" );
    ui->treeView->setModel(fsmodel);
    
    setUnifiedTitleAndToolBarOnMac(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}
