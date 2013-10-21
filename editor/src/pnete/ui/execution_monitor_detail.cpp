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
#include <QDateTimeEdit>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include <QTimer>

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

        for (int i (0); i < count; ++i)
        {
          setHeaderData ( _column_count + i
                        , Qt::Horizontal
                        , QVariant::fromValue (name_column)
                        , column_type_role
                        );
        }

        _column_count += count;

        endInsertColumns();
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

        case visible_range_from_role:
          fhg_assert (_column_types[index] == gantt_column, "visible range only defined for gantt columns");
          return QVariant::fromValue (_visible_ranges[index].from);

        case visible_range_to_role:
          fhg_assert (_column_types[index] == gantt_column, "visible range only defined for gantt columns");
          return QVariant::fromValue (_visible_ranges[index].to);

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
        else if (role == visible_range_from_role || role == visible_range_to_role)
        {
          fhg_assert (_column_types[index] == gantt_column, "visible range only defined for gantt columns");
          if (util::qt::stores<QDateTime> (variant))
          {
            const long val ( util::qt::value<QDateTime> (variant)
                           .toMSecsSinceEpoch()
                           - util::qt::value<QDateTime>
                             (index.data (worker_model::base_time_role))
                           .toMSecsSinceEpoch()
                           );

            if (_visible_ranges.contains (index))
            {
              ( role == visible_range_from_role
              ? _visible_ranges[index].from
              : _visible_ranges[index].to
              ) = val;
            }
            else
            {
              _visible_ranges[index] = visible_range_type (val);
            }
          }
          else
          {
            if (_visible_ranges.contains (index))
            {
              ( role == visible_range_from_role
              ? _visible_ranges[index].from
              : _visible_ranges[index].to
              ) = util::qt::value<long> (variant);
            }
            else
            {
              _visible_ranges[index] =
                visible_range_type (util::qt::value<long> (variant));
            }
          }
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

      namespace
      {
        void move_with_constant_range
          (util::qt::mvc::section_index index, long t)
        {
          const execution_monitor_proxy::visible_range_type r
            ( util::qt::value<execution_monitor_proxy::visible_range_type>
              (index.data (execution_monitor_proxy::visible_range_role))
            );

          index.data
            ( QVariant::fromValue
              (execution_monitor_proxy::visible_range_type (t - r.length(), t))
            , execution_monitor_proxy::visible_range_role
            );
        }
      }

      void execution_monitor_proxy::move_tick()
      {
        BOOST_FOREACH (const util::qt::mvc::section_index& index, _auto_moving)
        {
          move_with_constant_range
            (index, util::qt::value<long> (index.data (execution_monitor_proxy::elapsed_time_role)));
        }
      }

      namespace
      {
        template<typename T> T sorted (T t) { qSort (t); return t; }

        bool intersects_or_touches (const QRectF& lhs, const QRectF& rhs)
        {
          return lhs.right() >= rhs.left() && rhs.right() >= lhs.left();
        }

        qreal right_plus (const QRectF& rect, qreal add)
        {
          return rect.right() + add;
        }

        QRectF shrunken_by_pen (QRectF rect, QPen pen)
        {
          const qreal by (pen.width() == 0 ? 1.0 : pen.widthF());
          return rect.adjusted (0.0, 0.0, -by, -by);
        }

        void widen (QRectF* rect, qreal by)
        {
          rect->setWidth (rect->width() + by);
        }

        const bool do_antialiasing (false);
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
            const QList<worker_model::subrange_getter_type> subrange_getters
              ( util::qt::collect<worker_model::subrange_getter_type>
                (index.data (worker_model::range_getter_role))
              );
            fhg_assert (!subrange_getters.empty(), "gantt requires at least one subrange getter");

            const util::qt::painter_state_saver state_saver (painter);
            painter->setClipRect (option.rect);

            const execution_monitor_proxy::visible_range_type visible_range
              ( util::qt::value<execution_monitor_proxy::visible_range_type>
                (section_index.data (execution_monitor_proxy::visible_range_role))
              );

            const bool distribute_vertically (subrange_getters.size() > 1);

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

            const qreal horizontal_scale
              (qreal (option.rect.width()) / qreal (visible_range.length()));
            const qreal y_pos (option.rect.top());
            const qreal height
              ( distribute_vertically
              ? option.rect.height()
              / qreal (sdpa::daemon::NotificationEvent::STATE_MAX + 1)
              : option.rect.height()
              );

            QHash<worker_model::state_type, QVector<QRectF> > paint_descriptions;
            QHash<worker_model::state_type, QVector<QLineF> > merge_lines;

            const bool merge_away_small_intervals (true || distribute_vertically);

            BOOST_FOREACH
              (worker_model::subrange_getter_type range, subrange_getters)
            {
              BOOST_FOREACH
                ( const worker_model::value_type& data
                , range (visible_range.from, visible_range.to)
                )
              {
                const qreal left (std::max (visible_range.from, data.timestamp()));
                QRectF rect ( qreal (option.rect.x())
                            + (left - visible_range.from) * horizontal_scale
                            , y_pos
                            , ( ( data.duration()
                                ? std::min ( visible_range.to
                                           , data.timestamp() + *data.duration()
                                           )
                                : visible_range.to
                                )
                              - left
                              ) * horizontal_scale
                            , height
                            );

                QVector<QRectF>& descr (paint_descriptions[data.state()]);

                static const qreal merge_threshold (2.0);

                if (merge_away_small_intervals)
                {
                  QVector<QRectF>::iterator inter
                    ( std::lower_bound
                      ( descr.begin(), descr.end()
                      , rect
                      , boost::bind (right_plus, _1, merge_threshold)
                      < boost::bind (&QRectF::left, _2)
                      )
                    );

                  while (inter != descr.end())
                  {
                    widen (&*inter, merge_threshold);
                    if (!intersects_or_touches (*inter, rect))
                    {
                      widen (&*inter, -merge_threshold);
                      break;
                    }
                    widen (&*inter, -merge_threshold);

                    //! \note if not actually intersecting, add line
                    if (rect.right() <= inter->left())
                    {
                      const qreal x ((rect.right() + inter->left()) / 2.0);
                      merge_lines[data.state()]
                        << QLineF (x, y_pos, x, y_pos + height);
                    }
                    else if (inter->right() <= rect.left())
                    {
                      const qreal x ((inter->right() + rect.left()) / 2.0);
                      merge_lines[data.state()]
                        << QLineF (x, y_pos, x, y_pos + height);
                    }

                    rect.setRight (qMax (rect.right(), inter->right()));
                    rect.setLeft (qMin (rect.left(), inter->left()));

                    inter = descr.erase (inter);
                  }

                  descr.insert (inter, shrunken_by_pen (rect, painter->pen()));
                }
                else
                {
                  descr.push_back (shrunken_by_pen (rect, painter->pen()));
                }
              }
            }

            BOOST_FOREACH ( const worker_model::state_type state
                          , sorted (paint_descriptions.keys())
                          )
            {
              painter->setBrush (color_for_state (state));

              painter->drawRects (paint_descriptions[state]);
              painter->drawLines (merge_lines[state]);

              if (distribute_vertically)
              {
                painter->translate (0.0, height);
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

      bool execution_monitor_delegate::helpEvent ( QHelpEvent *event
                                                 , QAbstractItemView *view
                                                 , const QStyleOptionViewItem& option
                                                 , const QModelIndex& index
                                                 )
      {
        //! \todo Implement tooltips showing information.
        return QAbstractItemDelegate::helpEvent(event, view, option, index);

        // if (!event || !view)
        // {
        //   return false;
        // }

        // if (event->type() == QEvent::ToolTip)
        // {
        //   QHelpEvent *he (static_cast<QHelpEvent*> (event));
        //   he->pos() - option.rect.topLeft();
        //   QVariant tooltip (index.data (Qt::ToolTipRole));
        //   if (util::qt::stores<QString> (tooltip))
        //   {
        //     QToolTip::showText (he->globalPos(), tooltip.toString(), view);
        //     return true;
        //   }
        // }
      }

      execution_monitor_editor::execution_monitor_editor
        ( execution_monitor_delegate* delegate
        , util::qt::mvc::section_index index
        , QWidget* parent
        )
          : QWidget (parent)
          , _scrollbar (new QScrollBar (Qt::Horizontal, this))
          , _lower (new QDateTimeEdit (this))
          , _upper (new QDateTimeEdit (this))
          , _automove (new QCheckBox (tr ("end = now()"), this))
          , _index (index)
      {
        const QDateTime base_time
          (util::qt::value<QDateTime> (index.data (worker_model::base_time_role)));

        _lower->setMinimumDateTime (base_time);
        _upper->setMinimumDateTime (base_time);
        update_maximum();
        update();

        _lower->setDisplayFormat ("dd.MM.yyyy hh:mm:ss.zzz");
        _upper->setDisplayFormat ("dd.MM.yyyy hh:mm:ss.zzz");


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


        util::qt::boost_connect<void (QDateTime)>
          ( _lower, SIGNAL (dateTimeChanged (QDateTime))
          , delegate, boost::bind ( &util::qt::mvc::section_index::data
                                  , _index
                                  , _1
                                  , execution_monitor_proxy::visible_range_from_role
                                  )
          );

        util::qt::boost_connect<void (QDateTime)>
          ( _upper, SIGNAL (dateTimeChanged (QDateTime))
          , delegate, boost::bind ( &util::qt::mvc::section_index::data
                                  , _index
                                  , _1
                                  , execution_monitor_proxy::visible_range_to_role
                                  )
          );

        util::qt::boost_connect<void (int)>
          ( _scrollbar, SIGNAL (valueChanged (int))
          , delegate, boost::bind (move_with_constant_range, _index, _1)
          );


        QTimer* timer (new QTimer (this));
        connect (timer, SIGNAL (timeout()), SLOT (update_maximum()));
        timer->start();


        new QHBoxLayout (this);
        layout()->addWidget (_scrollbar);
        layout()->addWidget (_automove);
        layout()->addWidget (_lower);
        layout()->addWidget (_upper);
      }

      void execution_monitor_editor::update_maximum()
      {
        const QDateTime now (QDateTime::currentDateTime());
        _lower->setMaximumDateTime (now);
        _upper->setMaximumDateTime (now);
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
          util::qt::scoped_signal_block block (_lower);
          _lower->setDateTime ( QDateTime::fromMSecsSinceEpoch
                                (base_time.toMSecsSinceEpoch() + visible_range.from)
                              );
        }
        {
          util::qt::scoped_signal_block block (_upper);
          _upper->setDateTime ( QDateTime::fromMSecsSinceEpoch
                                (base_time.toMSecsSinceEpoch() + visible_range.to)
                              );
        }
        {
          util::qt::scoped_signal_block block (_scrollbar);
          _scrollbar->setValue (visible_range.to);
          _scrollbar->setMinimum (visible_range.length());
          _scrollbar->setPageStep (visible_range.length());
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
        }

        //! \note Is asserted above.
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

            const long& from (range.from);
            const long& to (range.to);
            const long visible_range (range.length());

            const qreal scale (qreal (rect.width()) / (to - from));
            const long available_pixels (rect.width());
            const long minimum_pixels_per_tick (15);
            const long maximum_ticks (available_pixels / minimum_pixels_per_tick);
            const long small_tick_interval (visible_range / maximum_ticks / 10 * 10);
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
