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

#include <pnete/ui/execution_monitor_worker_model.hpp>

#include <util/qt/mvc/fixed_proxy_models.hpp>
#include <util/qt/mvc/header_delegate.hpp>
#include <util/qt/mvc/section_index.hpp>

#include <QStyledItemDelegate>

#include <functional>

class QCheckBox;
class QSpinBox;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class execution_monitor_proxy : public util::qt::mvc::id_proxy
      {
        Q_OBJECT

      public:
        enum roles
        {
          //! \note worker_model has first three
          visible_range_role = Qt::UserRole + 100,
          visible_range_to_role,
          visible_range_length_role,
          automatically_move_role,
          elapsed_time_role,
          column_type_role,
          merge_groups_role,
        };

        struct visible_range_type
        {
          long _to;
          long _length;
          long to() const
          {
            return _to;
          }
          long from() const
          {
            return _to - _length;
          }
          void to (long t) { _to = t; }
          long length() const
          {
            return _length;
          }
          void length (long l) { _length = l; }
          visible_range_type (long t = 0)
            : _to (t)
            , _length (1)
          { }
          visible_range_type (long t, long length)
            : _to (t)
            , _length (length)
          { }
        };

        enum column_type
        {
          name_column,
          gantt_column,
          current_states_column
        };

        execution_monitor_proxy (QAbstractItemModel* model, QObject* parent = nullptr);

        QVariant headerData
          (int section, Qt::Orientation, int role = Qt::DisplayRole) const override;
        bool setHeaderData
          (int section, Qt::Orientation, QVariant const&, int role = Qt::EditRole) override;

        void setSourceModel (QAbstractItemModel*) override;

        int columnCount (QModelIndex const& = QModelIndex()) const override;
        QModelIndex mapToSource (QModelIndex const& proxy) const override;
        QModelIndex index (int, int, QModelIndex const&) const override;
        bool insertColumns
          (int column, int count, QModelIndex const& parent = QModelIndex()) override;
        bool removeColumns
          (int column, int count, QModelIndex const& parent = QModelIndex()) override;

      private slots:
        void move_tick();
        void source_dataChanged (QModelIndex const&, QModelIndex const&);

      private:
        using util::qt::mvc::id_proxy::setSourceModel;

        void move_existing_columns (int begin, int offset);

        QMap<util::qt::mvc::section_index, visible_range_type> _visible_ranges;
        QSet<util::qt::mvc::section_index> _auto_moving;
        QMap<util::qt::mvc::section_index, column_type> _column_types;
        QSet<util::qt::mvc::section_index> _merge_groups;

        int _column_count {1};
      };

      class execution_monitor_delegate : public QStyledItemDelegate
                                       , public util::qt::mvc::header_delegate
      {
        Q_OBJECT

      public:
        execution_monitor_delegate ( std::function<void (QString)> set_filter
                                   , std::function<QString()> get_filter
                                   , QMap<worker_model::state_type, QColor>
                                   , QWidget* parent = nullptr
                                   );

        void paint ( QPainter* painter
                           , QStyleOptionViewItem const& option
                           , QModelIndex const& index
                           ) const override;

        void paint
          (QPainter*, QRect const&, util::qt::mvc::section_index const&) override;
        QWidget* create_editor ( QRect const&
                                       , util::qt::mvc::delegating_header_view*
                                       , util::qt::mvc::section_index const&
                                       ) override;
        void release_editor
          (util::qt::mvc::section_index const&, QWidget* editor) override;
        void update_editor
          (util::qt::mvc::section_index, QWidget* editor) override;
        bool can_edit_section (util::qt::mvc::section_index) const override;
        QMenu* menu_for_section (util::qt::mvc::section_index) const override;
        void wheel_event (util::qt::mvc::section_index, QWheelEvent*) override;

        QColor color_for_state (worker_model::state_type) const;

      signals:
        void color_for_state_changed
          (sdpa::daemon::NotificationEvent::state_t, QColor);

      public slots:
        void color_for_state (worker_model::state_type, QColor);

        bool helpEvent ( QHelpEvent* event
                               , QAbstractItemView* view
                               , QStyleOptionViewItem const& option
                               , QModelIndex const& index
                               ) override;

      private:
        std::function<void (QString)> _set_filter;
        std::function<QString()> _get_filter;

        QMap<worker_model::state_type, QColor> _color_for_state;
      };

      class execution_monitor_editor : public QWidget
      {
        Q_OBJECT

      public:
        execution_monitor_editor ( execution_monitor_delegate*
                                 , util::qt::mvc::section_index
                                 , QWidget* parent = nullptr
                                 );

      public slots:
        void update();
        void update_maximum();

      private:
        QScrollBar* _scrollbar;
        QSpinBox* _visible_range_length;
        QCheckBox* _automove;
        QCheckBox* _merge_groups;

        util::qt::mvc::section_index _index;
      };
    }
  }
}

//! \note visible_range_role
Q_DECLARE_METATYPE (fhg::pnete::ui::execution_monitor_proxy::visible_range_type)
//! \note column_type_role
Q_DECLARE_METATYPE (fhg::pnete::ui::execution_monitor_proxy::column_type)
