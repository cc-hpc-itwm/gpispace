#ifndef MONITORWINDOW_HPP
#define MONITORWINDOW_HPP

#include <QMainWindow>

#include <fhglog/remote/LogServer.hpp>
#include <fhglog/Appender.hpp>
#include <fhglog/fhglog.hpp>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include "portfolioeval.hpp"
#include <sdpa/daemon/NotificationEvent.hpp>

#include <we/we.hpp>

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

// portfolio evaluation
public:

public slots:
    void clearLogging();
    void toggleFollowLogging(bool checked);
    void levelFilterChanged (int lvl);
    // portfolio related slots
    void ClearTable() { m_portfolio_.ClearTable(); }
    void SubmitPortfolio() { m_portfolio_.SubmitPortfolio(); }
    void resizePortfolio(int k) { m_portfolio_.Resize(k); }

  // execution view
  void clearActivityLog();

private:
    bool event (QEvent *event);

  void UpdatePortfolioView( sdpa::daemon::NotificationEvent const & evt
                          , we::activity_t const & act
                          );

  void UpdateExecutionView( std::string const & host
                          , sdpa::daemon::NotificationEvent const & evt
                          , we::activity_t const & act
                          );

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

    Portfolio m_portfolio_;

  std::map<std::string, size_t> m_node_to_row;
  std::map<std::string, std::pair<size_t, size_t> > m_activity_to_cell;
  size_t m_current_col;
};

#endif // MONITORWINDOW_HPP
