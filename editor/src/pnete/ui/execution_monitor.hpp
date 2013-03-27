#ifndef FHG_PNETE_UI_EXECUTION_MONITOR_HPP
#define FHG_PNETE_UI_EXECUTION_MONITOR_HPP

#include <fhglog/fhglog.hpp>
#include <fhglog/remote/LogServer.hpp>

#include <sdpa/daemon/NotificationEvent.hpp>

#include <boost/thread.hpp>

#include <QWidget>
#include <QTimer>

class Task;
class QGraphicsItem;
class QGraphicsView;
class QGraphicsScene;

class execution_monitor : public QWidget
{
  Q_OBJECT;

public:
  explicit execution_monitor (unsigned short port, QWidget* parent = NULL);
  ~execution_monitor();

  void append_exe (const fhg::log::LogEvent &);

public slots:
  void toggleFollowTaskView (bool);
  void toggle_automatically_sort_components (bool);
  void sort_gantt_by_component();

  void clearActivityLog();
  void advance();
  void changeTaskViewZoom (int);

  void change_gantt_color (const QString&);

private:
  void handle_external_event (const fhg::log::LogEvent &);

  bool m_follow_execution;

  QGraphicsScene* m_scene;
  QGraphicsView* m_view;

  QGraphicsScene* m_component_scene;
  QGraphicsView* m_component_view;

  QTimer _advance_timer;

  qreal m_current_scale;

  mutable boost::recursive_mutex _scene_updates_lock;
  typedef std::pair<QGraphicsItem*, QGraphicsScene*> scene_update_t;
  typedef std::pair<QGraphicsItem*, std::string> component_update_t;
  std::list<scene_update_t> _scene_updates;
  std::list<component_update_t> _component_updates;

  mutable boost::recursive_mutex m_task_struct_mutex;
  std::vector<std::string> m_components;
  std::map<std::string, std::map<std::string, Task*> > m_tasks_grid;
  std::map<std::string, std::list<Task*> > m_tasks_list;
  std::map<std::string, QGraphicsItem*> _component_labels;
  std::map<std::string, QGraphicsItem*> _component_dummies;

  bool _automatically_sort_components;
  bool _sort_gantt_trigger;

  boost::asio::io_service m_io_service;
  fhg::log::remote::LogServer m_exe_server;
  boost::thread m_io_thread;
};

#endif
