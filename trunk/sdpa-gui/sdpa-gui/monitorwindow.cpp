#include "monitorwindow.hpp"
#include "ui_monitorwindow.h"

MonitorWindow::MonitorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MonitorWindow)
{
    ui->setupUi(this);
}

MonitorWindow::~MonitorWindow()
{
    delete ui;
}
