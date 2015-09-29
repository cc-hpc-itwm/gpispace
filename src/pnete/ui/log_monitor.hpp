#pragma once

#include <fhglog/LogMacros.hpp>
#include <fhglog/remote/server.hpp>

#include <boost/thread.hpp>

#include <sdpa/daemon/NotificationEvent.hpp>

#include <QAbstractTableModel>
#include <QMutex>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QTableView;

namespace detail
{
  struct formatted_log_event
  {
    QString time;
    QString source;
    QString message;

    fhg::log::LogEvent event;

    formatted_log_event (const fhg::log::LogEvent& evt);
  };

  class log_table_model : public QAbstractTableModel
  {
    Q_OBJECT

  public:
    log_table_model (QObject* parent = nullptr);

    virtual int rowCount (const QModelIndex& = QModelIndex()) const override;
    virtual int columnCount (const QModelIndex& = QModelIndex()) const override;

    virtual QVariant headerData ( int section
                                , Qt::Orientation orientation
                                , int role = Qt::DisplayRole
                                ) const override;
    virtual QVariant
      data (const QModelIndex& index, int role = Qt::DisplayRole) const override;

    std::vector<fhg::log::LogEvent> data() const;
    void add (const fhg::log::LogEvent& event);

  public slots:
    void clear();
    void update();

  private:
    QList<formatted_log_event> _pending_data;
    bool _clear_on_update;
    mutable QMutex _mutex_pending;

    QList<formatted_log_event> _data;
    mutable QMutex _mutex_data;
  };

  class log_filter_proxy;
}

class log_monitor : public QWidget
{
  Q_OBJECT

public:
  explicit log_monitor (unsigned short port, QWidget* parent = nullptr);
  ~log_monitor();

public slots:
  void toggle_follow_logging (bool);
  void save();

private:
  void append_log_event (const fhg::log::LogEvent &);

  bool _drop_filtered;
  int _filter_level;

  QTableView* _log_table;
  detail::log_table_model* _log_model;
  detail::log_filter_proxy* _log_filter;

  QThread* _log_model_update_thread;
  QTimer* _log_model_update_timer;

  QString _last_saved_filename;

  boost::asio::io_service _io_service;
  fhg::log::Logger _logger;
  fhg::log::remote::LogServer _log_server;
  boost::thread _io_thread;
};
