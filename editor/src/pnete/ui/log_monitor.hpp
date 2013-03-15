#ifndef FHG_PNETE_UI_LOG_MONITOR_HPP
#define FHG_PNETE_UI_LOG_MONITOR_HPP

#include <fhglog/fhglog.hpp>
#include <fhglog/remote/LogServer.hpp>

#include <boost/thread.hpp>

#include <sdpa/daemon/NotificationEvent.hpp>

#include <QWidget>

class QCheckBox;
class QComboBox;
class QTableView;

namespace detail
{
  class log_table_model;
  class log_filter_proxy;
}

class log_monitor : public QWidget
{
  Q_OBJECT;

public:
  explicit log_monitor (unsigned short port, QWidget* parent = NULL);
  ~log_monitor();

public slots:
  void clearLogging();
  void toggleFollowLogging (bool);
  void levelFilterChanged (int);
  void save();

private:
  void handle_external_event (const fhg::log::LogEvent &);

  bool event (QEvent *event);

  boost::asio::io_service m_io_service;
  fhg::log::remote::LogServer m_log_server;
  boost::thread m_io_thread;
  std::vector<fhg::log::LogEvent> m_log_events;

  QString m_logfile;

  QCheckBox* m_drop_filtered;
  QComboBox* m_level_filter_selector;
  QTableView* m_log_table;

  detail::log_table_model* _log_model;
  detail::log_filter_proxy* _log_filter;
};

#endif
