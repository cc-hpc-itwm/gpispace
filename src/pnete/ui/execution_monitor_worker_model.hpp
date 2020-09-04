// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/range.hpp>

#include <QAbstractItemModel>
#include <QDateTime>
#include <QVector>

#include <functional>
#include <mutex>
#include <vector>

namespace fhg
{
  namespace pnete
  {
    namespace ui
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

        void append_event (logging::message const&);

        virtual int rowCount (const QModelIndex& = QModelIndex()) const override;
        virtual int columnCount (const QModelIndex& = QModelIndex()) const override;
        virtual QVariant data
          (const QModelIndex&, int role = Qt::DisplayRole) const override;
        virtual QVariant headerData
          (int section, Qt::Orientation, int role = Qt::DisplayRole) const override;
        virtual QModelIndex index
          (int row, int column, const QModelIndex& = QModelIndex()) const override;
        virtual QModelIndex parent (const QModelIndex&) const override;

        void clear();
        virtual bool removeRows
          (int row, int count, const QModelIndex& parent = QModelIndex()) override;

        typedef long timestamp_type;
        typedef long duration_type;
        typedef sdpa::daemon::NotificationEvent::state_t state_type;

        struct value_type
        {
          value_type ( timestamp_type, boost::optional<duration_type>
                     , QString id, QString name, state_type
                     );

          timestamp_type timestamp() const;
          boost::optional<duration_type> duration() const;

          QString id() const;

          state_type state() const;

        private:
          void duration (boost::optional<duration_type> d);
          void state (state_type state_, timestamp_type now);

          timestamp_type _timestamp;
          boost::optional<duration_type> _duration;

          QString _id;
          QString _name;
          state_type _state;

          friend class worker_model;
        };

        typedef boost::iterator_range<std::vector<value_type>::const_iterator>
          subrange_type;
        typedef std::function<subrange_type (timestamp_type, timestamp_type)>
          subrange_getter_type;

      private slots:
        void handle_events();

      private:
        QList<QString> _workers;
        QMap<QString, std::vector<value_type>> _worker_containers;
        QDateTime _base_time;

        std::mutex _event_queue;
        QVector<logging::message> _queued_events;
      };
    }
  }
}

//! \note current_interval_role
Q_DECLARE_METATYPE (boost::optional<fhg::pnete::ui::worker_model::value_type>)
//! \note range_getter_role
Q_DECLARE_METATYPE (fhg::pnete::ui::worker_model::subrange_getter_type)
