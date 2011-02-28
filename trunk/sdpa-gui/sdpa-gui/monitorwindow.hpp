#ifndef MONITORWINDOW_HPP
#define MONITORWINDOW_HPP

#include <QMainWindow>

namespace Ui {
    class MonitorWindow;
}

class MonitorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MonitorWindow(QWidget *parent = 0);
    ~MonitorWindow();

private:
    Ui::MonitorWindow *ui;
};

#endif // MONITORWINDOW_HPP
