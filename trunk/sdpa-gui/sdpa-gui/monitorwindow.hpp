#ifndef MONITORWINDOW_HPP
#define MONITORWINDOW_HPP

#include <QMainWindow>

#include <fhglog/remote/LogServer.hpp>
#include <fhglog/Appender.hpp>
#include <fhglog/fhglog.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

namespace Ui {
    class MonitorWindow;
}

class MonitorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MonitorWindow( unsigned short exe_port
                          , unsigned short log_port
                          , QWidget *parent = 0
                          );
    ~MonitorWindow();
    void append_log (const fhg::log::LogEvent &);
    void append_exe (const fhg::log::LogEvent &);
public slots:
    void clearLogging ();
    void toggleFollowLogging(bool checked);
    void levelFilterChanged (int lvl);

private:
    bool event (QEvent *event);

    Ui::MonitorWindow *ui;
    typedef boost::shared_ptr<boost::thread> thread_t;
    typedef boost::shared_ptr<fhg::log::remote::LogServer> logserver_t;
    boost::asio::io_service m_io_service;
    thread_t m_io_thread;
    logserver_t m_log_server;
    logserver_t m_exe_server;
    bool m_follow_logging;
    bool m_follow_execution;
    std::vector<fhg::log::LogEvent> m_log_events;
};

#endif // MONITORWINDOW_HPP
