// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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

    virtual int rowCount (QModelIndex const& = QModelIndex()) const override;
    virtual int columnCount (QModelIndex const& = QModelIndex()) const override;

    virtual QVariant headerData ( int section
                                , Qt::Orientation orientation
                                , int role = Qt::DisplayRole
                                ) const override;
    virtual QVariant
      data (QModelIndex const& index, int role = Qt::DisplayRole) const override;

    std::vector<fhg::logging::message> data() const;
    void add (fhg::logging::message);

  public slots:
    void clear();
    void update();

  private:
    QList<formatted_log_event> _pending_data;
    bool _clear_on_update;
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
  bool _drop_filtered;
  int _filter_level;

  QTableView* _log_table;
  detail::log_table_model* _log_model;
  detail::log_filter_proxy* _log_filter;

  QThread* _log_model_update_thread;
  QTimer* _log_model_update_timer;

  QString _last_saved_filename;
};
