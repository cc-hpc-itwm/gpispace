// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/execution_monitor_detail.hpp>

#include <util/qt/boost_connect.hpp>
#include <util/qt/cast.hpp>
#include <util/qt/mvc/delegating_header_view.hpp>
#include <util/qt/painter_state_saver.hpp>
#include <util/qt/scoped_disconnect.hpp>
#include <util/qt/scoped_signal_block.hpp>
#include <util/qt/variant.hpp>

#include <fhg/assert.hpp>
#include <fhg/util/backtracing_exception.hpp>

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

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      execution_monitor_proxy::execution_monitor_proxy (QAbstractItemModel* model, QObject* parent)
        : util::qt::mvc::id_proxy (parent)
        , _column_count (1)
      {
        setSourceModel (model);

        connect ( this, SIGNAL (dataChanged (QModelIndex,QModelIndex))
                , SLOT (source_dataChanged (QModelIndex,QModelIndex))
                );

        setHeaderData ( 0, Qt::Horizontal
                      , QVariant::fromValue (name_column), column_type_role
                      );

        QTimer* timer (new QTimer (this));
        connect (timer, SIGNAL (timeout()), SLOT (move_tick()));
        static int fps (30);
        timer->start (1000/fps);
      }

      namespace
      {
        void abort_on_column_change()
        {
          throw util::backtracing_exception ("column count of source model changed, which should not happen");
        }
      }

      void execution_monitor_proxy::setSourceModel (QAbstractItemModel* model)
      {
        if (sourceModel())
        {
          sourceModel()->disconnect (this);
        }
        util::qt::mvc::id_proxy::setSourceModel (model);
        if (sourceModel())
        {
          fhg_assert (sourceModel()->columnCount() == 1, "source model shall only have one column");
          util::qt::boost_connect<void ()>
            ( sourceModel()
            , SIGNAL (columnsAboutToBeInserted (const QModelIndex&, int, int))
            , &abort_on_column_change
            );
          util::qt::boost_connect<void ()>
            ( sourceModel()
            , SIGNAL (columnsAboutToBeRemoved (const QModelIndex&, int, int))
            , &abort_on_column_change
            );
        }
      }

      void execution_monitor_proxy::move_existing_columns (int begin, int offset)
      {
        for (int i (begin); i < _column_count; ++i)
        {
          const util::qt::mvc::section_index index
            (this, Qt::Horizontal, i);
          const util::qt::mvc::section_index new_index
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
        (const QModelIndex& from, const QModelIndex& to)
      {
        const util::qt::scoped_disconnect disconnecter
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

      int execution_monitor_proxy::columnCount (const QModelIndex&) const
      {
        return _column_count;
      }

      QModelIndex execution_monitor_proxy::index (int r, int c, const QModelIndex& p) const
      {
        if (r >= rowCount (p) || c >= columnCount (p))
        {
          return QModelIndex();
        }

        const QModelIndex base (id_proxy::index (r, 0, p));
        return createIndex (base.row(), c, base.internalPointer());
      }
      QModelIndex execution_monitor_proxy::mapToSource (const QModelIndex& proxy) const
      {
        return id_proxy::mapToSource
          (createIndex (proxy.row(), 0, proxy.internalPointer()));
      }

      bool execution_monitor_proxy::insertColumns
        (int column, int count, const QModelIndex& parent)
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
        (int column, int count, const QModelIndex& parent)
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
          const util::qt::mvc::section_index index (this, Qt::Horizontal, i);
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
        const util::qt::mvc::section_index index (this, orientation, section);

        switch (role)
        {
        case visible_range_role:
          fhg_assert (_column_types[index] == gantt_column, "visible range only defined for gantt columns");
          return QVariant::fromValue (_visible_ranges[index]);

        case visible_range_to_role:
          fhg_assert (_column_types[index] == gantt_column, "visible range only defined for gantt columns");
          return QVariant::fromValue (_visible_ranges[index].to());

        case visible_range_length_role:
          fhg_assert (_column_types[index] == gantt_column, "visible range only defined for gantt columns");
          return QVariant::fromValue (_visible_ranges[index].length());

        case automatically_move_role:
          fhg_assert (_column_types[index] == gantt_column, "automatically moving only defined for gantt columns");
          return _auto_moving.contains (index);

        case elapsed_time_role:
          return QVariant::fromValue<long>
            ( QDateTime::currentDateTime().toMSecsSinceEpoch()
            - util::qt::value<QDateTime>
              ( util::qt::mvc::id_proxy::headerData
                (section, orientation, worker_model::base_time_role)
              ).toMSecsSinceEpoch()
            );

        case column_type_role:
          return QVariant::fromValue (_column_types[index]);
        }

        return util::qt::mvc::id_proxy::headerData (section, orientation, role);
      }

      bool execution_monitor_proxy::setHeaderData ( int section
                                                  , Qt::Orientation orientation
                                                  , const QVariant& variant
                                                  , int role
                                                  )
      {
        const util::qt::mvc::section_index index (this, orientation, section);

        if (role == visible_range_role)
        {
          fhg_assert (_column_types[index] == gantt_column, "visible range only defined for gantt columns");
          _visible_ranges[index] = util::qt::value<visible_range_type> (variant);
        }
        else if (role == visible_range_to_role)
        {
          fhg_assert (_column_types[index] == gantt_column, "visible range only defined for gantt columns");
          _visible_ranges[index].to ( util::qt::stores<QDateTime> (variant)
                                    ? ( util::qt::value<QDateTime> (variant)
                                      .toMSecsSinceEpoch()
                                      - util::qt::value<QDateTime>
                                      (index.data (worker_model::base_time_role))
                                      .toMSecsSinceEpoch()
                                      )
                                    : util::qt::value<int> (variant)
                                    );
        }
        else if (role == visible_range_length_role)
        {
          fhg_assert (_column_types[index] == gantt_column, "visible range only defined for gantt columns");
          _visible_ranges[index].length (util::qt::value<int> (variant));
        }
        else if (role == automatically_move_role)
        {
          fhg_assert (_column_types[index] == gantt_column, "automatically moving only defined for gantt columns");
          if (util::qt::value<bool> (variant))
          {
            _auto_moving.insert (index);
          }
          else
          {
            _auto_moving.remove (index);
          }
        }
        else if (role == column_type_role)
        {
          const column_type value (util::qt::value<column_type> (variant));
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
              break;
            }
            _column_types[index] = value;
            switch (value)
            {
            case name_column:
            case current_states_column:
              break;

            case gantt_column:
              _visible_ranges.insert (index, visible_range_type (0, 1000 * 60));
              _auto_moving.insert (index);
              break;
            }
          }
        }
        else
        {
          return util::qt::mvc::id_proxy::setHeaderData
            (section, orientation, variant, role);
        }

        emit headerDataChanged (orientation, section, section);
        return true;
      }

      void execution_monitor_proxy::move_tick()
      {
        BOOST_FOREACH (const util::qt::mvc::section_index& index, _auto_moving)
        {
          index.data ( QDateTime::currentDateTime()
                     , execution_monitor_proxy::visible_range_to_role
                     );
        }
      }

      execution_monitor_delegate::execution_monitor_delegate
        ( boost::function<void (QString)> set_filter
        , boost::function<QString()> get_filter
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
      void execution_monitor_delegate::color_for_state
        (worker_model::state_type state, QColor color)
      {
        _color_for_state[state] = color;
        //! \todo This would require the view to be updated, but _how_?
        emit color_for_state_changed (state, color);
      }

      namespace
      {
        struct paint_description
        {
          struct block
          {
            QRectF rect;
            QStringList subranges;
            block (QRectF r, QString range)
              : rect (r)
            {
              subranges.push_back (range);
            }
            block() {}
          };
          QHash<worker_model::state_type, QVector<block> > blocks;
          bool distribute_vertically;
          qreal height;

          paint_description (bool distr, qreal h)
            : distribute_vertically (distr)
            , height (h)
          {
            blocks[sdpa::daemon::NotificationEvent::STATE_STARTED];
            blocks[sdpa::daemon::NotificationEvent::STATE_FINISHED];
            blocks[sdpa::daemon::NotificationEvent::STATE_FAILED];
            blocks[sdpa::daemon::NotificationEvent::STATE_CANCELLED];
          }
        };

        template<typename T> T sorted (T t) { qSort (t); return t; }

        bool intersects_or_touches (const QRectF& lhs, const QRectF& rhs)
        {
          return lhs.right() >= rhs.left() && rhs.right() >= lhs.left();
        }

        bool overlaps ( const paint_description::block& block
                      , const QRectF& rect
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
          QRectF* _rect;
          qreal _by;
        };

        const bool do_antialiasing (false);

        paint_description prepare_gantt_row ( QModelIndex index
                                            , QRect rect
                                            , QPen pen
                                            )
        {
          const util::qt::mvc::section_index section_index
            (index, Qt::Horizontal);

          const QList<worker_model::subrange_getter_type> subrange_getters
            ( util::qt::collect<worker_model::subrange_getter_type>
              (index.data (worker_model::range_getter_role))
            );
          fhg_assert (!subrange_getters.empty(), "gantt requires at least one subrange getter");

          const execution_monitor_proxy::visible_range_type visible_range
            ( util::qt::value<execution_monitor_proxy::visible_range_type>
              (section_index.data (execution_monitor_proxy::visible_range_role))
            );
          const long from (visible_range.from());
          const long to (visible_range.to());

          const bool distribute_vertically (subrange_getters.size() > 1);

          paint_description descr
            ( distribute_vertically
            , distribute_vertically
            ? rect.height()
            / qreal (sdpa::daemon::NotificationEvent::STATE_MAX + 1)
            : rect.height()
            );

          const bool merge_away_small_intervals
            (true || descr.distribute_vertically);

          const qreal horizontal_scale
            (qreal (rect.width()) / qreal (visible_range.length()));

          BOOST_FOREACH
            (worker_model::subrange_getter_type range, subrange_getters)
          {
            BOOST_FOREACH (const worker_model::value_type& data, range (from, to))
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
                , data.id()
                );

              QVector<paint_description::block>& blocks_in_state
                (descr.blocks[data.state()]);

              static const qreal merge_threshold (2.0);

              if (merge_away_small_intervals)
              {
                QVector<paint_description::block>::iterator inter
                  ( std::lower_bound
                    ( blocks_in_state.begin(), blocks_in_state.end()
                    , block.rect
                    , boost::bind (&overlaps, _1, _2, merge_threshold)
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
                  block.subranges += inter->subranges;

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
                                             , const QStyleOptionViewItem& option
                                             , const QModelIndex& index
                                             ) const
      {
        const util::qt::mvc::section_index section_index
          (index, Qt::Horizontal);

        switch ( util::qt::value<execution_monitor_proxy::column_type>
                 (section_index.data (execution_monitor_proxy::column_type_role))
               )
        {
        case execution_monitor_proxy::name_column:
          QStyledItemDelegate::paint (painter, option, index);
          break;

        case execution_monitor_proxy::gantt_column:
          {
            const util::qt::painter_state_saver state_saver (painter);
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

            BOOST_FOREACH ( const worker_model::state_type state
                          , sorted (descr.blocks.keys())
                          )
            {
              painter->setBrush (color_for_state (state));

              BOOST_FOREACH
                (const paint_description::block& block, descr.blocks[state])
              {
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
            const QList<boost::optional<worker_model::value_type> > current_intervals
              ( util::qt::collect<boost::optional<worker_model::value_type> >
                (index.data (worker_model::current_interval_role))
              );

            const util::qt::painter_state_saver state_saver (painter);

            if (option.state & QStyle::State_Selected)
            {
              painter->fillRect (option.rect, option.palette.highlight());
            }

            painter->setPen ( option.state & QStyle::State_Selected
                            ? option.palette.highlightedText().color()
                            : option.palette.text().color()
                            );

            QHash<worker_model::state_type, int> in_state;

            BOOST_FOREACH ( boost::optional<worker_model::value_type> current
                          , current_intervals
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
            BOOST_FOREACH
              (const worker_model::state_type state, sorted (in_state.keys()))
            {
              x_pos += 3.0;
              const QString text (QString ("%1").arg (in_state[state]));
              const qreal text_width (QFontMetrics (painter->font()).width (text) + 4.0);
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
          typedef sdpa::daemon::NotificationEvent event;

          switch (state)
          {
          case event::STATE_STARTED: return "started";
          case event::STATE_FINISHED: return "finished";
          case event::STATE_FAILED: return "failed";
          case event::STATE_CANCELLED: return "cancelled";
          }
        }
      }

      bool maybe_show_tooltip
        ( const worker_model::state_type state
        , const paint_description& descr
        , QHelpEvent* event
        , QAbstractItemView* view
        )
      {
        BOOST_FOREACH (const paint_description::block& block, descr.blocks[state])
        {
          if ( block.rect.left() <= event->pos().x()
             && event->pos().x() <= block.rect.right()
             )
          {
            const QString ranges ( block.subranges.size() > 20
                                 ? QStringList (block.subranges.mid (0, 20)).join (", ") + "..."
                                 : block.subranges.join (", ")
                                 );
            QToolTip::showText
              ( event->globalPos()
              , to_string (state) + ": " + ranges
              , view
              );
            return true;
          }
        }
        return false;
      }

      bool execution_monitor_delegate::helpEvent
        ( QHelpEvent* event
        , QAbstractItemView* view
        , const QStyleOptionViewItem& option
        , const QModelIndex& index
        )
      {
        if (index.isValid() && event->type() == QEvent::ToolTip)
        {
          const util::qt::mvc::section_index section_index
            (index, Qt::Horizontal);

          if ( util::qt::value<execution_monitor_proxy::column_type>
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
              BOOST_FOREACH (worker_model::state_type state, descr.blocks.keys())
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
        , util::qt::mvc::section_index index
        , QWidget* parent
        )
          : QWidget (parent)
          , _scrollbar (new QScrollBar (Qt::Horizontal, this))
          , _visible_range_length (new QSpinBox (this))
          , _automove (new QCheckBox (tr ("end = now()"), this))
          , _index (index)
      {
        const QDateTime base_time
          (util::qt::value<QDateTime> (index.data (worker_model::base_time_role)));

        _visible_range_length->setMinimum (5000);
        _visible_range_length->setMaximum (INT_MAX);
        _visible_range_length->setSuffix (" msec");
        update_maximum();
        update();


        util::qt::boost_connect<void (int)>
          ( _scrollbar, SIGNAL (valueChanged (int))
          , _automove, boost::bind (&QCheckBox::setChecked, _automove, false)
          );

        util::qt::boost_connect<void (bool)>
          ( _automove, SIGNAL (toggled (bool))
          , delegate, boost::bind ( &util::qt::mvc::section_index::data
                                  , _index
                                  , _1
                                  , execution_monitor_proxy::automatically_move_role
                                  )
          );


        util::qt::boost_connect<void (int)>
          ( _visible_range_length, SIGNAL (valueChanged (int))
          , delegate, boost::bind ( &util::qt::mvc::section_index::data
                                  , _index
                                  , _1
                                  , execution_monitor_proxy::visible_range_length_role
                                  )
          );

        util::qt::boost_connect<void (int)>
          ( _scrollbar, SIGNAL (valueChanged (int))
          , delegate, boost::bind ( &util::qt::mvc::section_index::data
                                  , _index
                                  , _1
                                  , execution_monitor_proxy::visible_range_to_role
                                  )
          );


        QTimer* timer (new QTimer (this));
        connect (timer, SIGNAL (timeout()), SLOT (update_maximum()));
        timer->start();


        new QHBoxLayout (this);
        layout()->addWidget (_scrollbar);
        layout()->addWidget (_automove);
        layout()->addWidget (_visible_range_length);
      }

      void execution_monitor_editor::update_maximum()
      {
        _scrollbar->setMaximum
          (util::qt::value<long> (_index.data (execution_monitor_proxy::elapsed_time_role)));
      }

      void execution_monitor_editor::update()
      {
        const QDateTime base_time
          (util::qt::value<QDateTime> (_index.data (worker_model::base_time_role)));
        const execution_monitor_proxy::visible_range_type visible_range
          (util::qt::value<execution_monitor_proxy::visible_range_type> (_index.data (execution_monitor_proxy::visible_range_role)));

        {
          util::qt::scoped_signal_block block (_visible_range_length);
          _visible_range_length->setValue (visible_range.length());
        }
        {
          util::qt::scoped_signal_block block (_scrollbar);
          _scrollbar->setValue (visible_range.to());
          _scrollbar->setPageStep
            (std::min (static_cast<int> (visible_range.length()), _scrollbar->maximum()));
          _scrollbar->setSingleStep (_scrollbar->pageStep() / 20);
        }

        {
          util::qt::scoped_signal_block block (_automove);
          _automove->setChecked
            (_index.data (execution_monitor_proxy::automatically_move_role).toBool());
        }
      }

      bool execution_monitor_delegate::can_edit_section
        (util::qt::mvc::section_index index) const
      {
        const execution_monitor_proxy::column_type t
          ( util::qt::value<execution_monitor_proxy::column_type>
            (index.data (execution_monitor_proxy::column_type_role))
          );
        return t == execution_monitor_proxy::name_column || t == execution_monitor_proxy::gantt_column;
      }

      QWidget* execution_monitor_delegate::create_editor
        ( const QRect&
        , util::qt::mvc::delegating_header_view* parent
        , const util::qt::mvc::section_index& index
        )
      {
        fhg_assert (can_edit_section (index), "only create editors for editable sections");

        switch ( util::qt::value<execution_monitor_proxy::column_type>
                 (index.data (execution_monitor_proxy::column_type_role))
               )
        {
        case execution_monitor_proxy::name_column:
          {
            QLineEdit* line_edit (new QLineEdit (_get_filter(), parent));
            util::qt::boost_connect<void (QString)>
              (line_edit, SIGNAL (textChanged (QString)), _set_filter);
            return line_edit;
          }

        case execution_monitor_proxy::gantt_column:
          return new execution_monitor_editor (this, index, parent);

        case execution_monitor_proxy::current_states_column:
          //! \note asserted above, but throwing a warning in release builds
          throw std::runtime_error ("can't create editor for non-editable section");
        }
      }

      void execution_monitor_delegate::release_editor
        (const util::qt::mvc::section_index&, QWidget* editor)
      {
        delete editor;
      }

      void execution_monitor_delegate::update_editor
        (util::qt::mvc::section_index, QWidget* editor)
      {
        util::qt::throwing_qobject_cast<execution_monitor_editor*>
          (editor)->update();
      }

      namespace
      {
        const QString& format_for_distance (const long dist)
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
        , const QRect& rect
        , const util::qt::mvc::section_index& index
        )
      {
        switch ( util::qt::value<execution_monitor_proxy::column_type>
                 (index.data (execution_monitor_proxy::column_type_role))
               )
        {
        case execution_monitor_proxy::name_column:
          break;

        case execution_monitor_proxy::gantt_column:
          {
            const execution_monitor_proxy::visible_range_type range
              (util::qt::value<execution_monitor_proxy::visible_range_type>
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

            const QString& format (format_for_distance (large_tick_interval));

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
                                          - QFontMetrics (painter->font()).width (text) / 2
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
        (util::qt::mvc::section_index index) const
      {
        const execution_monitor_proxy::column_type type
          ( util::qt::value<execution_monitor_proxy::column_type>
            (index.data (execution_monitor_proxy::column_type_role))
          );

        if (type == execution_monitor_proxy::name_column)
        {
          fhg_assert (index._section == 0, "only column 0 should be a name column");
          return NULL;
        }

        QMenu* menu (new QMenu);

        QAction* toggle_column_type
          ( menu->addAction (tr ( type == execution_monitor_proxy::gantt_column
                                ? "change_to_current_states_column_action"
                                : "change_to_gantt_column_action"
                                )
                            )
          );
        fhg::util::qt::boost_connect<void()>
          ( toggle_column_type
          , SIGNAL (triggered())
          , index._model
          , boost::bind ( &util::qt::mvc::section_index::data
                        , index
                        , type == execution_monitor_proxy::gantt_column
                        ? QVariant::fromValue (execution_monitor_proxy::current_states_column)
                        : QVariant::fromValue (execution_monitor_proxy::gantt_column)
                        , execution_monitor_proxy::column_type_role
                        )
          );

        menu->addSeparator();

        QAction* remove (menu->addAction (tr ("remove_column_action")));
        fhg::util::qt::boost_connect<void()>
          ( remove
          , SIGNAL (triggered())
          , index._model
          , boost::bind ( index._orientation == Qt::Horizontal
                        ? &QAbstractItemModel::removeColumn
                        : &QAbstractItemModel::removeRow
                        , index._model
                        , index._section
                        , QModelIndex()
                        )
          );

        return menu;
      }
    }
  }
}
