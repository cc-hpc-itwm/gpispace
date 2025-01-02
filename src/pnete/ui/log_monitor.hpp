// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <logging/message.hpp>

#include <sdpa/daemon/NotificationEvent.hpp>

#include <QAbstractTableModel>
#include <QMutex>
#include <QWidget>

#include <vector>

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

    QColor color;
    int legacy_severity;

    fhg::logging::message _raw;

    formatted_log_event (fhg::logging::message);
  };

  class log_table_model : public QAbstractTableModel
  {
    Q_OBJECT

  public:
    log_table_model (QObject* parent = nullptr);

    int rowCount (QModelIndex const& = QModelIndex()) const override;
    int columnCount (QModelIndex const& = QModelIndex()) const override;

    QVariant headerData ( int section
                                , Qt::Orientation orientation
                                , int role = Qt::DisplayRole
                                ) const override;
    QVariant
      data (QModelIndex const& index, int role = Qt::DisplayRole) const override;

    std::vector<fhg::logging::message> data() const;
    void add (fhg::logging::message);

  public slots:
    void clear();
    void update();

  private:
    QList<formatted_log_event> _pending_data;
    bool _clear_on_update{};
    QMutex _mutex_pending;

    QList<formatted_log_event> _data;
    mutable QMutex _mutex_data;
  };

  class log_filter_proxy;
}

class log_monitor : public QWidget
{
  Q_OBJECT

public:
  log_monitor();
  ~log_monitor() override;
  log_monitor (log_monitor const&) = delete;
  log_monitor (log_monitor&&) = delete;
  log_monitor& operator= (log_monitor const&) = delete;
  log_monitor& operator= (log_monitor&&) = delete;

public slots:
  void toggle_follow_logging (bool);
  void save();
  void append_log_event (fhg::logging::message const&);

private:
  bool _drop_filtered {false};
  int _filter_level {1};

  QTableView* _log_table;
  detail::log_table_model* _log_model;
  detail::log_filter_proxy* _log_filter;

  QThread* _log_model_update_thread;
  QTimer* _log_model_update_timer;

  QString _last_saved_filename;
};
