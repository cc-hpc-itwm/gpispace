// Copyright (C) 2013-2016,2018-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/logging/message.hpp>

#include <gspc/scheduler/daemon/NotificationEvent.hpp>

#include <boost/asio.hpp>
#include <boost/range.hpp>

#include <QAbstractItemModel>
#include <QDateTime>
#include <QVector>

#include <functional>
#include <mutex>
#include <optional>
#include <vector>



    namespace gspc::monitor
    {
      class worker_model : public QAbstractItemModel
      {
        Q_OBJECT

      public:
        enum roles
        {
          name_role = Qt::UserRole,
          base_time_role,
          current_interval_role,
          range_getter_role
        };

        worker_model (QObject* parent);

        void append_event (gspc::logging::message const&);

        int rowCount (QModelIndex const& = QModelIndex()) const override;
        int columnCount (QModelIndex const& = QModelIndex()) const override;
        QVariant data
          (QModelIndex const&, int role = Qt::DisplayRole) const override;
        QVariant headerData
          (int section, Qt::Orientation, int role = Qt::DisplayRole) const override;
        QModelIndex index
          (int row, int column, QModelIndex const& = QModelIndex()) const override;
        QModelIndex parent (QModelIndex const&) const override;

        void clear();
        bool removeRows
          (int row, int count, QModelIndex const& parent = QModelIndex()) override;

        using timestamp_type = long;
        using duration_type = long;
        using state_type = gspc::scheduler::daemon::NotificationEvent::state_t;
        using ColorsForState = gspc::scheduler::daemon::NotificationEvent::ColorsForState;

        struct value_type
        {
          value_type ( timestamp_type, std::optional<duration_type>
                     , QString id, QString name, state_type
                     , ColorsForState colors
                     , std::optional<QString> tooltip
                     );

          timestamp_type timestamp() const;
          std::optional<duration_type> duration() const;

          QString id() const;
          QString name() const;

          state_type state() const;

          std::optional<QString> color_for_state (state_type) const;
          std::optional<QString> tooltip() const;

        private:
          void duration (std::optional<duration_type> d);
          void state (state_type state_, timestamp_type now);

          timestamp_type _timestamp;
          std::optional<duration_type> _duration;

          QString _id;
          QString _name;
          state_type _state {gspc::scheduler::daemon::NotificationEvent::STATE_STARTED};
          ColorsForState _colors;
          std::optional<QString> _tooltip;

          friend class worker_model;
        };

        using subrange_type =
          ::boost::iterator_range<std::vector<value_type>::const_iterator>;
        using subrange_getter_type =
          std::function<subrange_type (timestamp_type, timestamp_type)>;

      private slots:
        void handle_events();

      private:
        QList<QString> _workers;
        QMap<QString, std::vector<value_type>> _worker_containers;
        QDateTime _base_time;

        std::mutex _event_queue;
        QVector<gspc::logging::message> _queued_events;
      };
    }



//! \note current_interval_role
Q_DECLARE_METATYPE (std::optional<gspc::monitor::worker_model::value_type>)
//! \note range_getter_role
Q_DECLARE_METATYPE (gspc::monitor::worker_model::subrange_getter_type)
