#ifndef MONITORWINDOW_HPP
#define MONITORWINDOW_HPP

#include <QMainWindow>
#include <QtGui>

#include <fhglog/remote/LogServer.hpp>
#include <fhglog/Appender.hpp>
#include <fhglog/fhglog.hpp>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

//#include "portfolioeval.hpp"
#include "task.h"
#include <sdpa/daemon/NotificationEvent.hpp>

#include <we/we.hpp>

namespace Ui {
    class MonitorWindow;
}

class MonitorWindow : public QMainWindow
{
    Q_OBJECT

    typedef boost::recursive_mutex mutex_type;
  typedef boost::unique_lock<mutex_type> lock_type;

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
    void toggleFollowTaskView(bool checked);
    void levelFilterChanged (int lvl);
    // portfolio related slots
  void ClearTable() { } //m_portfolio_.ClearTable(); }
  void SubmitPortfolio() { } //m_portfolio_.SubmitPortfolio(); }
  void resizePortfolio(int k) { } //m_portfolio_.Resize(k); }

  // execution view
  void clearActivityLog();
  void advance();
  void changeTaskViewZoom(int);    // from slider

private:
  void handle_external_event (int type, const fhg::log::LogEvent &);

    bool event (QEvent *event);
  /*
  void UpdatePortfolioView( sdpa::daemon::NotificationEvent const & evt
                          , we::activity_t const & act
                          );
  */

  void UpdateExecutionView( sdpa::daemon::NotificationEvent const & evt
                          , we::activity_t const & act
                          );

  Ui::MonitorWindow *ui;
  mutable mutex_type m_task_view_mutex;
  mutable mutex_type m_task_struct_mutex;

    typedef boost::shared_ptr<boost::thread> thread_t;
    typedef boost::shared_ptr<fhg::log::remote::LogServer> logserver_t;
    boost::asio::io_service m_io_service;
    thread_t m_io_thread;
    logserver_t m_log_server;
    logserver_t m_exe_server;
    bool m_follow_logging;
    bool m_follow_execution;
    std::vector<fhg::log::LogEvent> m_log_events;

  //  Portfolio m_portfolio_;
  QGraphicsScene m_scene;
  QGraphicsView m_view;

  QGraphicsScene m_component_scene;
  QGraphicsView m_component_view;

  QTimer m_timer;

  qreal m_current_scale;

  typedef std::list<std::pair<QGraphicsItem*, QGraphicsScene*> > scene_update_list_t;
  scene_update_list_t m_scene_updates;

  std::vector<std::string> m_components;
  typedef std::map<std::string, Task*> id_to_task_map_t;
  typedef std::list<Task*> task_list_t;
  typedef std::map<std::string, id_to_task_map_t> component_to_task_map_t;
  typedef std::map<std::string, task_list_t> component_to_task_list_t;
  component_to_task_map_t m_tasks_grid;
  component_to_task_list_t m_tasks_list;
};

#endif // MONITORWINDOW_HPP
