#ifndef MONITORWINDOW_HPP
#define MONITORWINDOW_HPP

#ifndef Q_MOC_RUN

#include <QMainWindow>
#include <QtGui>

#include <fhglog/remote/LogServer.hpp>
#include <fhglog/Appender.hpp>
#include <fhglog/fhglog.hpp>

#include <boost/thread.hpp>

#include "task.h"
#include <sdpa/daemon/NotificationEvent.hpp>

#include <we/type/net.hpp> // recursive wrapper of transition_t fails otherwise.
#include <we/mgmt/type/activity.hpp>

#endif

class QGraphicsView;
class QGraphicsScene;

class MonitorWindow : public QMainWindow
{
  Q_OBJECT;

public:
  explicit MonitorWindow( unsigned short exe_port
                        , unsigned short log_port
                        , QWidget *parent = 0
                        );
  ~MonitorWindow();
  void append_log (const fhg::log::LogEvent &);
  void append_exe (const fhg::log::LogEvent &);

public slots:
  void clearLogging();
  void toggleFollowLogging(bool checked);
  void toggleFollowTaskView(bool checked);
  void levelFilterChanged (int lvl);
  void save ();

  // execution view
  void clearActivityLog();
  void advance();
  void changeTaskViewZoom(int);    // from slider

  void change_gantt_color (const QString&);

private:
  void handle_external_event (const fhg::log::LogEvent &);

  bool event (QEvent *event);
  void UpdateExecutionView
    (const sdpa::daemon::NotificationEvent&, const we::mgmt::type::activity_t&);

  boost::asio::io_service m_io_service;
  fhg::log::remote::LogServer m_log_server;
  fhg::log::remote::LogServer m_exe_server;
  boost::thread m_io_thread;
  bool m_follow_logging;
  bool m_follow_execution;
  std::vector<fhg::log::LogEvent> m_log_events;

  QGraphicsScene *m_scene;
  QGraphicsView *m_view;

  QGraphicsScene *m_component_scene;
  QGraphicsView *m_component_view;

  QTimer m_timer;

  qreal m_current_scale;

  mutable boost::recursive_mutex _scene_updates_lock;
  typedef std::pair<QGraphicsItem*, QGraphicsScene*> scene_update_t;
  std::list<scene_update_t> _scene_updates;

  mutable boost::recursive_mutex m_task_struct_mutex;
  std::vector<std::string> m_components;
  std::map<std::string, std::map<std::string, Task*> > m_tasks_grid;
  std::map<std::string, std::list<Task*> > m_tasks_list;

  QString m_logfile;

  QCheckBox* m_drop_filtered;
  QComboBox* m_level_filter_selector;
  QTableWidget* m_log_table;
};

#endif // MONITORWINDOW_HPP
