// Copyright (C) 2013-2017,2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/monitor/execution_monitor_detail.hpp>

#include <gspc/util/qt/compat.hpp>
#include <gspc/util/qt/overload.hpp>
#include <gspc/util/qt/painter_state_saver.hpp>
#include <gspc/util/qt/variant.hpp>
#include <gspc/util/qt/cast.hpp>
#include <gspc/util/qt/mvc/delegating_header_view.hpp>
#include <gspc/util/qt/scoped_disconnect.hpp>
#include <gspc/util/qt/scoped_signal_block.hpp>

#include <gspc/assert.hpp>
#include <gspc/util/backtracing_exception.hpp>

#include <QCheckBox>
#include <QHBoxLayout>
#include <QHelpEvent>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include <QSpinBox>
#include <QTimer>
#include <QToolTip>

#include <algorithm>
#include <stdexcept>



    namespace gspc::monitor
    {
      execution_monitor_proxy::execution_monitor_proxy (QAbstractItemModel* model, QObject* parent)
        : gspc::util::qt::mvc::id_proxy (parent)
      {
        setSourceModel (model);

        connect ( this, SIGNAL (dataChanged (QModelIndex,QModelIndex))
                , SLOT (source_dataChanged (QModelIndex,QModelIndex))
                );

        setHeaderData ( 0, Qt::Horizontal
                      , QVariant::fromValue (name_column), column_type_role
                      );

        auto* timer (new QTimer (this));
        connect (timer, SIGNAL (timeout()), SLOT (move_tick()));
        static int fps (30);
        timer->start (1000/fps);
      }

      namespace
      {
        [[noreturn]] void abort_on_column_change()
        {
          throw gspc::util::backtracing_exception ("column count of source model changed, which should not happen");
        }
      }

      void execution_monitor_proxy::setSourceModel (QAbstractItemModel* model)
      {
        if (sourceModel())
        {
          sourceModel()->disconnect (this);
        }
        gspc::util::qt::mvc::id_proxy::setSourceModel (model);
        if (sourceModel())
        {
          gspc_assert (sourceModel()->columnCount() == 1, "source model shall only have one column");
          connect
            ( sourceModel()
            , &QAbstractItemModel::columnsAboutToBeInserted
            , &abort_on_column_change
            );
          connect
            ( sourceModel()
            , &QAbstractItemModel::columnsAboutToBeRemoved
            , &abort_on_column_change
            );
        }
      }

      void execution_monitor_proxy::move_existing_columns (int begin, int offset)
      {
        for (int i (begin); i < _column_count; ++i)
        {
          const gspc::util::qt::mvc::section_index index
            (this, Qt::Horizontal, i);
          const gspc::util::qt::mvc::section_index new_index
            (this, Qt::Horizontal, i + offset);

          if (_visible_ranges.contains (index))
          {
            const visible_range_type value (_visible_ranges[index]);
            _visible_ranges.remove (index);
            _visible_ranges.insert (new_index, value);
          }
          if (_auto_moving.contains (index))
          {
            _auto_moving.remove (index);
            _auto_moving.insert (new_index);
          }
          if (_column_types.contains (index))
          {
            const column_type value (_column_types[index]);
            _column_types.remove (index);
            _column_types.insert (new_index, value);
          }
        }
      }

      void execution_monitor_proxy::source_dataChanged
        (QModelIndex const& from, QModelIndex const& to)
      {
        const gspc::util::qt::scoped_disconnect disconnecter
          ( this, SIGNAL (dataChanged (QModelIndex,QModelIndex))
          , this, SLOT (source_dataChanged (QModelIndex,QModelIndex))
          );

        for (int row (from.row()); row <= to.row(); ++row)
        {
          for (int col (1); col < _column_count; ++col)
          {
            const QModelIndex index
              (createIndex (row, col, from.sibling (row, 0).internalPointer()));
            emit dataChanged (index, index);
          }
        }
      }

      int execution_monitor_proxy::columnCount (QModelIndex const&) const
      {
        return _column_count;
      }

      QModelIndex execution_monitor_proxy::index (int r, int c, QModelIndex const& p) const
      {
        if (r >= rowCount (p) || c >= columnCount (p))
        {
          return QModelIndex();
        }

        const QModelIndex base (id_proxy::index (r, 0, p));
        return createIndex (base.row(), c, base.internalPointer());
      }
      QModelIndex execution_monitor_proxy::mapToSource (QModelIndex const& proxy) const
      {
        return id_proxy::mapToSource
          (createIndex (proxy.row(), 0, proxy.internalPointer()));
      }

      bool execution_monitor_proxy::insertColumns
        (int column, int count, QModelIndex const& parent)
      {
        if (parent.isValid())
        {
          return false;
        }

        beginInsertColumns (parent, column, column + count - 1);

        move_existing_columns (column + count, column + count - column);

        _column_count += count;

        endInsertColumns();

        for (int i (0); i < count; ++i)
        {
          setHeaderData ( _column_count + i
                        , Qt::Horizontal
                        , QVariant::fromValue (name_column)
                        , column_type_role
                        );
        }

        //! \note QSortFilterProxyModel (alphanum_sort) does not
        //! recursively invalidate cached columnCount(). Thus, force
        //! doing so by "changing the layout", which clears the cached
        //! mapping.
        emit layoutAboutToBeChanged();
        emit layoutChanged();

        return true;
      }


      bool execution_monitor_proxy::removeColumns
        (int column, int count, QModelIndex const& parent)
      {
        if (parent.isValid() || _column_count - count < 1)
        {
          return false;
        }

        const int from (column);
        const int to (column + count - 1);

        beginRemoveColumns (parent, from, to);

        for (int i (from); i <= to; ++i)
        {
          const gspc::util::qt::mvc::section_index index (this, Qt::Horizontal, i);
          _visible_ranges.remove (index);
          _auto_moving.remove (index);
          _column_types.remove (index);
        }

        move_existing_columns (to + 1, -(to - from + 1));

        _column_count -= count;

        endRemoveColumns();

        //! \note QSortFilterProxyModel (alphanum_sort) does not
        //! recursively invalidate cached columnCount(). Thus, force
        //! doing so by "changing the layout", which clears the cached
        //! mapping.
        emit layoutAboutToBeChanged();
        emit layoutChanged();

        return true;
      }

      QVariant execution_monitor_proxy::headerData
        (int section, Qt::Orientation orientation, int role) const
      {
        const gspc::util::qt::mvc::section_index index (this, orientation, section);

        switch (role)
        {
        case visible_range_role:
          gspc_assert (_column_types[index] == gantt_column, "visible range only defined for gantt columns");
          return QVariant::fromValue (_visible_ranges[index]);

        case visible_range_to_role:
          gspc_assert (_column_types[index] == gantt_column, "visible range only defined for gantt columns");
          return QVariant::fromValue (_visible_ranges[index].to());

        case visible_range_length_role:
          gspc_assert (_column_types[index] == gantt_column, "visible range only defined for gantt columns");
          return QVariant::fromValue (_visible_ranges[index].length());

        case automatically_move_role:
          gspc_assert (_column_types[index] == gantt_column, "automatically moving only defined for gantt columns");
          return _auto_moving.contains (index);

        case merge_groups_role:
          gspc_assert (_column_types[index] == gantt_column, "merge_groups only defined for gantt columns");
          return _merge_groups.contains (index);

        case elapsed_time_role:
          return QVariant::fromValue<long>
            ( QDateTime::currentDateTime().toMSecsSinceEpoch()
            - gspc::util::qt::value<QDateTime>
              ( gspc::util::qt::mvc::id_proxy::headerData
                (section, orientation, worker_model::base_time_role)
              ).toMSecsSinceEpoch()
            );

        case column_type_role:
          return QVariant::fromValue (_column_types[index]);
        }

        return gspc::util::qt::mvc::id_proxy::headerData (section, orientation, role);
      }

      bool execution_monitor_proxy::setHeaderData ( int section
                                                  , Qt::Orientation orientation
                                                  , QVariant const& variant
                                                  , int role
                                                  )
      {
        const gspc::util::qt::mvc::section_index index (this, orientation, section);

        if (role == visible_range_role)
        {
          gspc_assert (_column_types[index] == gantt_column, "visible range only defined for gantt columns");
          _visible_ranges[index] = gspc::util::qt::value<visible_range_type> (variant);
        }
        else if (role == visible_range_to_role)
        {
          gspc_assert (_column_types[index] == gantt_column, "visible range only defined for gantt columns");
          _visible_ranges[index].to ( gspc::util::qt::stores<QDateTime> (variant)
                                    ? ( gspc::util::qt::value<QDateTime> (variant)
                                      .toMSecsSinceEpoch()
                                      - gspc::util::qt::value<QDateTime>
                                      (index.data (worker_model::base_time_role))
                                      .toMSecsSinceEpoch()
                                      )
                                    : gspc::util::qt::value<int> (variant)
                                    );
        }
        else if (role == visible_range_length_role)
        {
          gspc_assert (_column_types[index] == gantt_column, "visible range only defined for gantt columns");
          _visible_ranges[index].length (gspc::util::qt::value<int> (variant));
        }
        else if (role == automatically_move_role)
        {
          gspc_assert (_column_types[index] == gantt_column, "automatically moving only defined for gantt columns");
          if (gspc::util::qt::value<bool> (variant))
          {
            _auto_moving.insert (index);
          }
          else
          {
            _auto_moving.remove (index);
          }
        }
        else if (role == merge_groups_role)
        {
          gspc_assert (_column_types[index] == gantt_column, "merge_groups only defined for gantt columns");
          if (gspc::util::qt::value<bool> (variant))
          {
            _merge_groups.insert (index);
          }
          else
          {
            _merge_groups.remove (index);
          }
        }
        else if (role == column_type_role)
        {
          const auto value (gspc::util::qt::value<column_type> (variant));
          if (_column_types[index] != value)
          {
            switch (_column_types[index])
            {
            case name_column:
            case current_states_column:
              break;

            case gantt_column:
              _visible_ranges.remove (index);
              _auto_moving.remove (index);
              _merge_groups.remove (index);
              break;
            }
            _column_types[index] = value;
            switch (value)
            {
            case name_column:
            case current_states_column:
              break;

            case gantt_column:
              _visible_ranges.insert (index, visible_range_type (0, 1000UL * 60UL));
              _auto_moving.insert (index);
              _merge_groups.insert (index);
              break;
            }
          }
        }
        else
        {
          return gspc::util::qt::mvc::id_proxy::setHeaderData
            (section, orientation, variant, role);
        }

        emit headerDataChanged (orientation, section, section);
        return true;
      }

      void execution_monitor_proxy::move_tick()
      {
        for (gspc::util::qt::mvc::section_index const& index : _auto_moving)
        {
          index.data ( QDateTime::currentDateTime()
                     , execution_monitor_proxy::visible_range_to_role
                     );
        }
      }

      execution_monitor_delegate::execution_monitor_delegate
        ( std::function<void (QString)> set_filter
        , std::function<QString()> get_filter
        , QMap<worker_model::state_type, QColor> color_for_state
        , QWidget* parent
        )
          : QStyledItemDelegate (parent)
          , _set_filter (set_filter)
          , _get_filter (get_filter)
          , _color_for_state (color_for_state)
      {
      }

      QColor execution_monitor_delegate::color_for_state
        (worker_model::state_type state) const
      {
        return _color_for_state[state];
      }

      namespace
      {
        struct paint_description
        {
          struct block
          {
            QRectF rect;
            QString name;
            std::optional<QString> tooltips;
            std::optional<QColor> custom_color;
            block ( QRectF r
                  , QString name
                  , std::optional<QString> tooltip
                  , std::optional<QString> color
                  )
              : rect (r)
              , name (name)
              , tooltips (tooltip)
              , custom_color (color ? std::optional<QColor> (QColor (*color)) : std::nullopt)
            {
            }
            block() = default;
          };
          QHash<worker_model::state_type, QVector<block>> blocks;
          bool distribute_vertically;
          qreal height;

          paint_description (bool distr, qreal h)
            : distribute_vertically (distr)
            , height (h)
          {
            blocks[gspc::scheduler::daemon::NotificationEvent::STATE_STARTED];
            blocks[gspc::scheduler::daemon::NotificationEvent::STATE_FINISHED];
            blocks[gspc::scheduler::daemon::NotificationEvent::STATE_FAILED];
            blocks[gspc::scheduler::daemon::NotificationEvent::STATE_CANCELED];
          }
        };

        template<typename T> T sorted (T t)
        {
          std::sort (t.begin(), t.end());
          return t;
        }

        bool intersects_or_touches (QRectF const& lhs, QRectF const& rhs)
        {
          return lhs.right() >= rhs.left() && rhs.right() >= lhs.left();
        }

        bool overlaps ( paint_description::block const& block
                      , QRectF const& rect
                      , qreal threshold
                      )
        {
          return block.rect.right() + threshold < rect.left();
        }

        QRectF shrunken_by_pen (QRectF rect, QPen pen)
        {
          const qreal by (pen.width() == 0 ? 1.0 : pen.widthF());
          return rect.adjusted (0.0, 0.0, -by, -by);
        }

        struct temporarily_widen
        {
          temporarily_widen (QRectF* rect, qreal by)
            : _rect (rect)
            , _by (by)
          {
            _rect->setWidth (_rect->width() + _by);
          }
          ~temporarily_widen()
          {
            _rect->setWidth (_rect->width() - _by);
          }
          temporarily_widen (temporarily_widen const&) = delete;
          temporarily_widen& operator= (temporarily_widen const&) = delete;
          temporarily_widen (temporarily_widen&&) = delete;
          temporarily_widen& operator= (temporarily_widen&&) = delete;

          QRectF* _rect;
          qreal _by;
        };

        const bool do_antialiasing (false);

        paint_description prepare_gantt_row ( QModelIndex index
                                            , QRect rect
                                            , QPen
                                            )
        {
          const gspc::util::qt::mvc::section_index section_index
            (index, Qt::Horizontal);

          const QList<worker_model::subrange_getter_type> subrange_getters
            ( gspc::util::qt::collect<worker_model::subrange_getter_type>
              (index.data (worker_model::range_getter_role))
            );
          gspc_assert (!subrange_getters.empty(), "gantt requires at least one subrange getter");

          const execution_monitor_proxy::visible_range_type visible_range
            ( gspc::util::qt::value<execution_monitor_proxy::visible_range_type>
              (section_index.data (execution_monitor_proxy::visible_range_role))
            );
          const long from (visible_range.from());
          const long to (visible_range.to());

          const bool distribute_vertically (subrange_getters.size() > 1);

          paint_description descr
            ( distribute_vertically
            , distribute_vertically
            ? rect.height()
            / qreal (gspc::scheduler::daemon::NotificationEvent::STATE_MAX + 1)
            : rect.height()
            );

          const bool merge_away_small_intervals
            (descr.distribute_vertically);

          const qreal horizontal_scale
            (qreal (rect.width()) / qreal (visible_range.length()));

          for (auto range : subrange_getters)
          {
            for (worker_model::value_type const& data : range (from, to))
            {
              const qreal left (std::max (from, data.timestamp()));
              paint_description::block block
                ( QRectF ( qreal (rect.x())
                         + (left - from) * horizontal_scale
                         , rect.top()
                         , ( ( data.duration()
                             ? std::min (to, data.timestamp() + *data.duration())
                             : to
                             )
                           - left
                           ) * horizontal_scale
                         , descr.height
                         )
                , data.name()
                , data.tooltip()
                , data.color_for_state (data.state())
                );

              QVector<paint_description::block>& blocks_in_state
                (descr.blocks[data.state()]);

              static const qreal merge_threshold (2.0);

              if (merge_away_small_intervals)
              {
                // Merged intervals use default colors and no tooltip
                block.custom_color.reset();
                block.tooltips.reset();

                QVector<paint_description::block>::iterator inter
                  ( std::lower_bound
                    ( blocks_in_state.begin(), blocks_in_state.end()
                    , block.rect
                    , std::bind (&overlaps, std::placeholders::_1, std::placeholders::_2, merge_threshold)
                    )
                  );

                while (inter != blocks_in_state.end())
                {
                  {
                    temporarily_widen _ (&inter->rect, merge_threshold);
                    if (!intersects_or_touches (inter->rect, block.rect))
                    {
                      break;
                    }
                  }

                  block.rect.setRight
                    (std::max (block.rect.right(), inter->rect.right()));
                  block.rect.setLeft
                    (std::min (block.rect.left(), inter->rect.left()));

                  inter = blocks_in_state.erase (inter);
                }

                blocks_in_state.insert (inter, block);
              }
              else
              {
                blocks_in_state.push_back (block);
              }
            }
          }
          return descr;
        }
      }

      void execution_monitor_delegate::paint ( QPainter* painter
                                             , QStyleOptionViewItem const& option
                                             , QModelIndex const& index
                                             ) const
      {
        const gspc::util::qt::mvc::section_index section_index
          (index, Qt::Horizontal);

        switch ( gspc::util::qt::value<execution_monitor_proxy::column_type>
                 (section_index.data (execution_monitor_proxy::column_type_role))
               )
        {
        case execution_monitor_proxy::name_column:
          QStyledItemDelegate::paint (painter, option, index);
          break;

        case execution_monitor_proxy::gantt_column:
          {
            if ( !gspc::util::qt::value<bool>
                   ( section_index.data
                       (execution_monitor_proxy::merge_groups_role)
                   )
               && index.model()->rowCount (index) != 0
               )
            {
              break;
            }

            const gspc::util::qt::painter_state_saver state_saver (painter);
            painter->setClipRect (option.rect);

            painter->setRenderHint (QPainter::Antialiasing, do_antialiasing);
            painter->setRenderHint (QPainter::TextAntialiasing, true);

            if (option.state & QStyle::State_Selected)
            {
              painter->fillRect (option.rect, option.palette.highlight());
            }
            painter->setPen ( option.state & QStyle::State_Selected
                            ? option.palette.highlightedText().color()
                            : option.palette.text().color()
                            );

            paint_description descr
              (prepare_gantt_row (index, option.rect, painter->pen()));

            for ( worker_model::state_type state
                : sorted (descr.blocks.keys())
                )
            {
              for (auto const& block : descr.blocks[state])
              {
                painter->setBrush
                  ( block.custom_color ? *block.custom_color
                                       : color_for_state (state)
                  );
                painter->drawRect (shrunken_by_pen (block.rect, painter->pen()));
              }

              if (descr.distribute_vertically)
              {
                painter->translate (0.0, descr.height);
              }
            }
          }

          break;

        case execution_monitor_proxy::current_states_column:
          {
            const QList<std::optional<worker_model::value_type>> current_intervals
              ( gspc::util::qt::collect<std::optional<worker_model::value_type>>
                (index.data (worker_model::current_interval_role))
              );

            const gspc::util::qt::painter_state_saver state_saver (painter);

            if (option.state & QStyle::State_Selected)
            {
              painter->fillRect (option.rect, option.palette.highlight());
            }

            painter->setPen ( option.state & QStyle::State_Selected
                            ? option.palette.highlightedText().color()
                            : option.palette.text().color()
                            );

            QHash<worker_model::state_type, int> in_state;

            for ( std::optional<worker_model::value_type> current
                : current_intervals
                )
            {
              if (current && !current->duration())
              {
                ++in_state[current->state()];
              }
            }

            const int inset (1);
            const QRectF rect_with_inset
              (option.rect.adjusted (inset, inset, -inset, -inset));

            qreal x_pos (0.0);
            for (auto state : sorted (in_state.keys()))
            {
              x_pos += 3.0;
              const QString text (QString ("%1").arg (in_state[state]));
              const qreal text_width
                (horizontal_advance (painter->font(), text) + 4.0);
              painter->drawText ( QRectF ( rect_with_inset.left() + x_pos
                                         , rect_with_inset.top()
                                         , text_width
                                         , rect_with_inset.height()
                                         )
                                , Qt::AlignCenter
                                , text
                                );
              x_pos += text_width;

              painter->setBrush (color_for_state (state));

              painter->drawRect ( QRectF ( rect_with_inset.left() + x_pos
                                         , rect_with_inset.top()
                                         , rect_with_inset.height()
                                         , rect_with_inset.height()
                                         )
                                );
              x_pos += rect_with_inset.height();
            }
          }

          break;
        }
      }

      namespace
      {
        QString to_string (worker_model::state_type state)
        {
          return QString::fromStdString
            (gspc::scheduler::daemon::NotificationEvent::to_string (state));
        }

        bool maybe_show_tooltip
          ( worker_model::state_type state
          , paint_description const& descr
          , QHelpEvent* event
          , QAbstractItemView* view
          )
        {
          for (paint_description::block const& block : descr.blocks[state])
          {
            if ( block.rect.left() <= event->pos().x()
               && event->pos().x() <= block.rect.right()
               )
            {
              QStringList tooltip_lines;

              tooltip_lines << "Task: " + block.name;
              tooltip_lines << "Status: " + to_string (state);

              if (block.tooltips)
              {
                tooltip_lines << "---";
                tooltip_lines << *block.tooltips;
              }

              QToolTip::showText
                ( event->globalPos()
                , tooltip_lines.join ("\n")
                , view
                );

              return true;
            }
          }
          return false;
        }
      }

      bool execution_monitor_delegate::helpEvent
        ( QHelpEvent* event
        , QAbstractItemView* view
        , QStyleOptionViewItem const& option
        , QModelIndex const& index
        )
      {
        if (index.isValid() && event->type() == QEvent::ToolTip)
        {
          const gspc::util::qt::mvc::section_index section_index
            (index, Qt::Horizontal);

          if ( gspc::util::qt::value<execution_monitor_proxy::column_type>
               (section_index.data (execution_monitor_proxy::column_type_role))
             ==  execution_monitor_proxy::gantt_column
             )
          {
            paint_description descr
              (prepare_gantt_row (index, option.rect, QPen()));

            if (descr.distribute_vertically)
            {
              if ( maybe_show_tooltip
                   ( descr.blocks.keys().at
                     ((event->pos().y() - option.rect.top()) / descr.height)
                   , descr, event, view
                   )
                 )
              {
                return true;
              }
            }
            else
            {
              for (auto state : descr.blocks.keys())
              {
                if (maybe_show_tooltip (state, descr, event, view))
                {
                  return true;
                }
              }
            }
          }
        }

        return QAbstractItemDelegate::helpEvent (event, view, option, index);
      }

      execution_monitor_editor::execution_monitor_editor
        ( execution_monitor_delegate* delegate
        , gspc::util::qt::mvc::section_index index
        , QWidget* parent
        )
          : QWidget (parent)
          , _scrollbar (new QScrollBar (Qt::Horizontal, this))
          , _visible_range_length (new QSpinBox (this))
          , _automove (new QCheckBox (tr ("end = now()"), this))
          , _merge_groups (new QCheckBox (tr ("merge_groups"), this))
          , _index (index)
      {
        const QDateTime base_time
          (gspc::util::qt::value<QDateTime> (index.data (worker_model::base_time_role)));

        _visible_range_length->setMinimum (5000);
        _visible_range_length->setMaximum (INT_MAX);
        _visible_range_length->setSuffix (" msec");
        update_maximum();
        update();


        connect
          ( _scrollbar, &QScrollBar::valueChanged
          , _automove, std::bind (&QCheckBox::setChecked, _automove, false)
          );

        connect
          ( _automove, &QCheckBox::toggled
          , delegate
          , [this] (bool value)
          {
            _index.data (value, execution_monitor_proxy::automatically_move_role);
          }
          );

        connect
          ( _visible_range_length, QOverload<int>::of (&QSpinBox::valueChanged)
          , delegate
          , [this] (int value)
          {
            _index.data (value, execution_monitor_proxy::visible_range_length_role);
          }
          );

        connect
          ( _scrollbar, &QScrollBar::valueChanged
          , delegate
          , [this] (int value)
          {
            _index.data (value, execution_monitor_proxy::visible_range_to_role);
          }
          );

        connect
          ( _merge_groups, &QCheckBox::toggled
          , delegate
          , [this] (bool value)
            {
              _index.data (value, execution_monitor_proxy::merge_groups_role);
            }
          );


        auto* timer (new QTimer (this));
        connect (timer, SIGNAL (timeout()), SLOT (update_maximum()));
        timer->start();


        new QHBoxLayout (this);
        layout()->addWidget (_scrollbar);
        layout()->addWidget (_automove);
        layout()->addWidget (_merge_groups);
        layout()->addWidget (_visible_range_length);
      }

      void execution_monitor_editor::update_maximum()
      {
        _scrollbar->setMaximum
          (gspc::util::qt::value<long> (_index.data (execution_monitor_proxy::elapsed_time_role)));
      }

      void execution_monitor_editor::update()
      {
        const QDateTime base_time
          (gspc::util::qt::value<QDateTime> (_index.data (worker_model::base_time_role)));
        const execution_monitor_proxy::visible_range_type visible_range
          (gspc::util::qt::value<execution_monitor_proxy::visible_range_type> (_index.data (execution_monitor_proxy::visible_range_role)));

        {
          gspc::util::qt::scoped_signal_block block (_visible_range_length);
          _visible_range_length->setValue (visible_range.length());
        }
        {
          gspc::util::qt::scoped_signal_block block (_scrollbar);
          _scrollbar->setValue (visible_range.to());
          _scrollbar->setPageStep
            (std::min (static_cast<int> (visible_range.length()), _scrollbar->maximum()));
          _scrollbar->setSingleStep (_scrollbar->pageStep() / 20);
        }

        {
          gspc::util::qt::scoped_signal_block block (_automove);
          _automove->setChecked
            (_index.data (execution_monitor_proxy::automatically_move_role).toBool());
        }

        {
          gspc::util::qt::scoped_signal_block block (_merge_groups);
          _merge_groups->setChecked
            (_index.data (execution_monitor_proxy::merge_groups_role).toBool());
        }
      }

      bool execution_monitor_delegate::can_edit_section
        (gspc::util::qt::mvc::section_index index) const
      {
        const auto t
          ( gspc::util::qt::value<execution_monitor_proxy::column_type>
            (index.data (execution_monitor_proxy::column_type_role))
          );
        return t == execution_monitor_proxy::name_column || t == execution_monitor_proxy::gantt_column;
      }

      QWidget* execution_monitor_delegate::create_editor
        ( QRect const&
        , gspc::util::qt::mvc::delegating_header_view* parent
        , gspc::util::qt::mvc::section_index const& index
        )
      {
        gspc_assert (can_edit_section (index), "only create editors for editable sections");

        const auto column
          ( gspc::util::qt::value<execution_monitor_proxy::column_type>
            (index.data (execution_monitor_proxy::column_type_role)
          ));
        switch (column)
        {
        case execution_monitor_proxy::name_column:
          {
            auto* line_edit (new QLineEdit (_get_filter(), parent));
            connect
              (line_edit, &QLineEdit::textChanged, _set_filter);
            return line_edit;
          }

        case execution_monitor_proxy::gantt_column:
          return new execution_monitor_editor (this, index, parent);

        case execution_monitor_proxy::current_states_column:
          //! \note asserted above, but throwing a warning in release builds
          throw std::runtime_error ("can't create editor for non-editable section");
        }

        throw std::logic_error {"invalid enum value"};
      }

      void execution_monitor_delegate::release_editor
        (gspc::util::qt::mvc::section_index const&, QWidget* editor)
      {
        delete editor;
      }

      void execution_monitor_delegate::update_editor
        (gspc::util::qt::mvc::section_index, QWidget* editor)
      {
        gspc::util::qt::throwing_qobject_cast<execution_monitor_editor*>
          (editor)->update();
      }

      namespace
      {
        QString const& format_for_distance (long dist)
        {
          const unsigned long abs_dist ( dist < 0
                                       ? static_cast<unsigned long> (-dist)
                                       : static_cast<unsigned long> (dist)
                                       );

#define CASE(distance, format)                  \
          if (abs_dist < distance)              \
          {                                     \
            static const QString fmt (format);  \
            return fmt;                         \
          }

          enum
          {
            ms = 1,
            s = 1000 * ms,
            min = 60 * s,
            hr = 60 * min,
            day = 24 * hr,
            week = 7 * day
            // with unsigned long as time_t, 24 days are a maximum,
            // thus no need for more
          };

          CASE (ms, "hh:mm:ss.zzz")
          CASE (s, "hh:mm:ss.zzz")
          CASE (min, "hh:mm:ss")
          CASE (hr, "ddd, hh:mm")
          CASE (day, "dd.MM., hh:mm")
          CASE (week, "dd.MM., hh:mm")

#undef CASE

          static const QString fallback ("dd.MM.yyyy hh:mm:ss.zzz");
          return fallback;
        }
      }

      void execution_monitor_delegate::paint
        ( QPainter* painter
        , QRect const& rect
        , gspc::util::qt::mvc::section_index const& index
        )
      {
        switch ( gspc::util::qt::value<execution_monitor_proxy::column_type>
                 (index.data (execution_monitor_proxy::column_type_role))
               )
        {
        case execution_monitor_proxy::name_column:
          break;

        case execution_monitor_proxy::gantt_column:
          {
            const execution_monitor_proxy::visible_range_type range
              (gspc::util::qt::value<execution_monitor_proxy::visible_range_type>
                (index.data (execution_monitor_proxy::visible_range_role))
              );

            painter->setRenderHint (QPainter::Antialiasing, do_antialiasing);
            painter->setRenderHint (QPainter::TextAntialiasing, true);

            const long from (range.from());
            const long to (range.to());
            const long visible_range (range.length());

            const qreal scale (qreal (rect.width()) / visible_range);
            const long available_pixels (rect.width());
            const long minimum_pixels_per_tick (15);
            const long maximum_ticks (available_pixels / minimum_pixels_per_tick);
            const long small_tick_interval (std::max (3L, visible_range / maximum_ticks / 10 * 10));
            const long large_tick_interval (small_tick_interval * 10);

            QString const& format (format_for_distance (large_tick_interval));

            for ( double i (from - from % large_tick_interval - large_tick_interval)
                ; i < to + large_tick_interval
                ; i += large_tick_interval
                )
            {
              painter->setPen (QPen (Qt::black, 2));
              painter->drawLine ( QLineF ( (i - from) * scale + rect.left()
                                         , rect.bottom() - rect.height() * 0.3
                                         , (i - from) * scale + rect.left()
                                         , rect.bottom()
                                         )
                                );

              painter->setPen (QPen (Qt::black, 1));
              const QString text
                ( QDateTime::fromMSecsSinceEpoch
                  ( index._model->headerData
                    ( index._section
                    , index._orientation
                    , worker_model::base_time_role
                    ).toDateTime().toMSecsSinceEpoch()
                  + i
                  ).toString (format)
                );
              painter->drawText ( QPointF ( (i - from) * scale + rect.left()
                                          - horizontal_advance (painter->font(), text) / 2
                                          , rect.bottom() - rect.height() * 0.35
                                          )
                                , text
                                );
            }

            painter->setPen (QPen (Qt::black, 1));
            for ( double i (from - from % small_tick_interval - small_tick_interval)
                ; i < to + small_tick_interval
                ; i += small_tick_interval
                )
            {
              painter->drawLine ( QLineF ( (i - from) * scale + rect.left()
                                         , rect.bottom() - rect.height() * 0.2
                                         , (i - from) * scale + rect.left()
                                         , rect.bottom()
                                         )
                                );
            }
          }
          break;

        case execution_monitor_proxy::current_states_column:
          break;
        }
      }

      QMenu* execution_monitor_delegate::menu_for_section
        (gspc::util::qt::mvc::section_index index) const
      {
        const auto type
          ( gspc::util::qt::value<execution_monitor_proxy::column_type>
            (index.data (execution_monitor_proxy::column_type_role))
          );

        if (type == execution_monitor_proxy::name_column)
        {
          gspc_assert (index._section == 0, "only column 0 should be a name column");
          return nullptr;
        }

        auto* menu (new QMenu);

        QAction* toggle_column_type
          ( menu->addAction (tr ( type == execution_monitor_proxy::gantt_column
                                ? "change_to_current_states_column_action"
                                : "change_to_gantt_column_action"
                                )
                            )
          );
        connect
          ( toggle_column_type
          , &QAction::triggered
          , index._model
          , [index, type]
          {
            index.data ( type == execution_monitor_proxy::gantt_column
                       ? QVariant::fromValue (execution_monitor_proxy::current_states_column)
                       : QVariant::fromValue (execution_monitor_proxy::gantt_column)
                       , execution_monitor_proxy::column_type_role
                       );
          }
          );

        menu->addSeparator();

        QAction* remove (menu->addAction (tr ("remove_column_action")));
        connect
          ( remove
          , &QAction::triggered
          , index._model
          , std::bind ( index._orientation == Qt::Horizontal
                      ? &QAbstractItemModel::removeColumn
                      : &QAbstractItemModel::removeRow
                      , index._model
                      , index._section
                      , QModelIndex()
                      )
          );

        return menu;
      }

      void execution_monitor_delegate::wheel_event
        (gspc::util::qt::mvc::section_index index, QWheelEvent* event)
      {
        index.data (false, execution_monitor_proxy::automatically_move_role);
        index.data ( static_cast<int>
                     ( gspc::util::qt::value<long>
                       (index.data (execution_monitor_proxy::visible_range_to_role))
                     )
                   + event->delta() * 10
                   , execution_monitor_proxy::visible_range_to_role
                   );
        event->accept();
      }
    }
