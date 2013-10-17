// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/gspc_monitor_detail.hpp>

#include <util/qt/boost_connect.hpp>
#include <util/qt/file_line_edit.hpp>

#include <fhg/assert.hpp>
#include <fhg/util/alphanum.hpp>
#include <fhg/util/num.hpp>
#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/require.hpp>
#include <fhg/util/read_bool.hpp>

#include <QApplication>
#include <QCheckBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSpinBox>
#include <QSplitter>
#include <QStyle>
#include <QToolTip>
#include <QVBoxLayout>
#include <QDateTime>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>

#include <iostream>
#include <sstream>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace
      {
        const qreal item_size (30.0);
        const qreal pen_size (0.0);
        const qreal padding (0.0);

        const qreal per_step (item_size + padding);
        const qreal node_size (item_size - pen_size);
      }

      state_description::state_description ( const QStringList& actions
                                           , boost::optional<char> character
                                           , QColor brush
                                           , QColor pen
                                           )
        : _actions (actions)
        , _character (character)
        , _brush (brush)
        , _pen (pen)
        , _hidden (false)
      {
        reset();
      }

      void state_description::reset()
      {
        _pixmap = QPixmap (node_size, node_size);

        QPainter painter (&_pixmap);
        painter.setRenderHint (QPainter::Antialiasing);

        painter.setBrush (_brush);
        painter.setPen (QPen (_pen, pen_size));

        painter.drawRect (0, 0, node_size, node_size);

        if (_character)
        {
          painter.drawText
            (0, 0, node_size, node_size, Qt::AlignCenter, QString (*_character));
        }
      }

      // ------------------------------------------------------------------------

      legend::legend (monitor_client* monitor_client, QWidget* parent)
        : QWidget (parent)
      {
        new QVBoxLayout (this);

        connect ( this
                , SIGNAL (state_pixmap_changed (const QString&))
                , SLOT (update (const QString&))
                );

        connect ( monitor_client
                , SIGNAL (states_add (const QString&, const QStringList&))
                , SLOT (states_add (const QString&, const QStringList&)));
        connect ( monitor_client
                , SIGNAL (states_layout_hint_border (const QString&, const QColor&))
                , SLOT (states_layout_hint_border (const QString&, const QColor&)));
        connect ( monitor_client
                , SIGNAL (states_layout_hint_character (const QString&, const char&))
                , SLOT (states_layout_hint_character (const QString&, const char&)));
        connect ( monitor_client
                , SIGNAL (states_layout_hint_color (const QString&, const QColor&))
                , SLOT (states_layout_hint_color (const QString&, const QColor&)));
        connect ( monitor_client
                , SIGNAL (states_layout_hint_hidden (const QString&, const bool&))
                , SLOT (states_layout_hint_hidden (const QString&, const bool&)));
        connect ( monitor_client
                , SIGNAL (states_layout_hint_descriptive_name (const QString&, const QString&))
                , SLOT (states_layout_hint_descriptive_name (const QString&, const QString&)));
      }

      void legend::states_add (const QString& state, const QStringList& actions)
      {
        _states.insert (state, state_description (actions));
        update (state);
      }

      void legend::states_layout_hint_border
        (const QString& state, const QColor& col)
      {
        state_description& desc (_states[state]);
        desc._pen = col;
        desc.reset();
        emit state_pixmap_changed (state);
      }
      void legend::states_layout_hint_character
        (const QString& state, const char& ch)
      {
        state_description& desc (_states[state]);
        desc._character = ch;
        desc.reset();
        emit state_pixmap_changed (state);
      }
      void legend::states_layout_hint_color
        (const QString& state, const QColor& col)
      {
        state_description& desc (_states[state]);
        desc._brush = col;
        desc.reset();
        emit state_pixmap_changed (state);
      }

      void legend::states_layout_hint_hidden
        (const QString& state, const bool& val)
      {
        _states[state]._hidden = val;
        update (state);
      }

      void legend::states_layout_hint_descriptive_name
        (const QString& state, const QString& val)
      {
        _states[state]._descriptive_name = val;
        update (state);
      }

      namespace
      {
        QWidget* legend_entry
          (const QString& name, const state_description& desc, QWidget* parent)
        {
          QWidget* widget (new QWidget (parent));
          QGridLayout* layout (new QGridLayout (widget));
          QLabel* label (new QLabel (widget));
          label->setPixmap (desc._pixmap);
          layout->addWidget (label, 0, 0);
          layout->addWidget (new QLabel (name, widget), 0, 1);
          return widget;
        }
      }

      void legend::update (const QString& s)
      {
        delete _state_legend[s];
        _state_legend[s] = NULL;
        if (!state (s)._hidden)
        {
          _state_legend[s] = legend_entry (state (s)._descriptive_name.get_value_or (s), state (s), this);
          layout()->addWidget (_state_legend[s]);
        }
      }

      const state_description&
        legend::state (const boost::optional<QString>& name) const
      {
        if (name && _states.find (*name) != _states.end())
        {
          return *_states.find (*name);
        }
        else
        {
          static state_description fallback (QStringList(), '?');
          return fallback;
        }
      }

      // ------------------------------------------------------------------------

      log_widget::log_widget (QWidget* parent)
        : QListWidget (parent)
      { }

      void log_widget::information (const QString& message)
      {
        new QListWidgetItem
          ( QApplication::style()->standardIcon (QStyle::SP_MessageBoxInformation)
          , QDateTime::currentDateTime().toString("hh:mm:ss") + ": " + message
          , this
          );
      }

      void log_widget::warning (const QString& message)
      {
        new QListWidgetItem
          ( QApplication::style()->standardIcon (QStyle::SP_MessageBoxWarning)
          , QDateTime::currentDateTime().toString("hh:mm:ss") + ": " + message
          , this
          );
      }

      void log_widget::critical (const QString& message)
      {
        new QListWidgetItem
          ( QApplication::style()->standardIcon (QStyle::SP_MessageBoxCritical)
          , QDateTime::currentDateTime().toString("hh:mm:ss") + ": " + message
          , this
          );
      }

      void log_widget::follow (bool follow)
      {
        if (!follow)
        {
          return;
        }

        scrollToBottom();

        QTimer* log_follower (new QTimer (this));

        connect (log_follower, SIGNAL (timeout()), this, SLOT (scrollToBottom()));
        connect (sender(), SIGNAL (toggled (bool)), log_follower, SLOT (stop()));
        connect (sender(), SIGNAL (toggled (bool)), log_follower, SLOT (deleteLater()));

        //! \todo Configurable refresh rate.
        static const int refresh_rate (20 /*ms*/);
        log_follower->start (refresh_rate);
      }

      // ------------------------------------------------------------------------

      namespace
      {
        const qreal base_coord (padding + pen_size / 2.0);

        int items_per_row (int width)
        {
          return (qMax (width - 2 * padding, per_step)) / per_step;
        }

        QRect rect_for_node (const int node, const int per_row)
        {
          return QRect ( (node % per_row) * per_step + base_coord
                       , (node / per_row) * per_step + base_coord
                       , node_size
                       , node_size
                       );
        }

        QTimer* timer (QObject* parent, int timeout, const char* slot)
        {
          QTimer* timer (new QTimer (parent));
          QObject::connect (timer, SIGNAL (timeout()), parent, slot);
          timer->start (timeout);
          return timer;
        }
        QTimer* timer (QObject* parent, int timeout, boost::function<void()> fun)
        {
          QTimer* timer (new QTimer (parent));
          fhg::util::qt::boost_connect<void()>
            (timer, SIGNAL (timeout()), parent, fun);
          timer->start (timeout);
          return timer;
        }
      }

      node_state_widget::node_state_widget
        ( const QString& window_title
        , legend* legend_widget
        , log_widget* log
        , monitor_client* monitor_client_
        , QWidget* parent
        )
          : QWidget (parent)
          , _legend_widget (legend_widget)
          , _log (log)
          , _monitor_client (monitor_client_)
          , _window_title (window_title)
      {
        timer (this, 30000, boost::bind (&monitor_client::request_hostlist, _monitor_client));
        timer (this, 1000, SLOT (refresh_stati()));

        setSizeIncrement (per_step, per_step);
        QSizePolicy pol (QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        pol.setHeightForWidth (true);
        setSizePolicy (pol);

        {
          QAction* select_all_nodes (new QAction (this));
          select_all_nodes->setShortcuts (QKeySequence::SelectAll);
          connect (select_all_nodes, SIGNAL (triggered()), SLOT (select_all()));
          addAction (select_all_nodes);
        }

        {
          QAction* clear_selection (new QAction (this));
          clear_selection->setShortcuts (QList<QKeySequence>() << Qt::Key_Escape);
          connect (clear_selection, SIGNAL (triggered()), SLOT (clear_selection()));
          addAction (clear_selection);
        }

        connect ( _monitor_client
                , SIGNAL (nodes (QStringList))
                , SLOT (nodes (QStringList)));
        connect ( _monitor_client
                , SIGNAL (nodes_details (const QString&, const boost::optional<QString>&))
                , SLOT (nodes_details (const QString&, const boost::optional<QString>&)));
        connect ( _monitor_client
                , SIGNAL (nodes_state (const QString&, const boost::optional<QString>&))
                , SLOT (nodes_state (const QString&, const boost::optional<QString>&)));
        connect ( _monitor_client
                , SIGNAL (states_actions_long_text (const QString&, const QString&))
                , SLOT (states_actions_long_text (const QString&, const QString&)));
        connect ( _monitor_client
                , SIGNAL (states_actions_requires_confirmation (const QString&, bool))
                , SLOT (states_actions_requires_confirmation (const QString&, bool)));
        connect ( _monitor_client
                , SIGNAL (states_actions_arguments (const QString&, const QList<action_argument_data>&))
                , SLOT (states_actions_arguments (const QString&, const QList<action_argument_data>&)));
        connect ( _monitor_client
                , SIGNAL (states_actions_expected_next_state (const QString&, const QString&))
                , SLOT (states_actions_expected_next_state (const QString&, const QString&)));
        connect ( _monitor_client
                , SIGNAL (action_result (const QString&, const QString&, const action_result_code&, const boost::optional<QString>&, QMap<QString, QString>))
                , SLOT (action_result (const QString&, const QString&, const action_result_code&, const boost::optional<QString>&, QMap<QString, QString>))
                );

        connect ( _legend_widget
                , SIGNAL (state_pixmap_changed (const QString&))
                , SLOT (update_nodes_with_state (const QString&))
                );

        _monitor_client->request_possible_status();
        _monitor_client->request_hostlist();
      }

      void node_state_widget::update (int node)
      {
        const int per_row (items_per_row (width()));

        QToolTip::hideText();

        QWidget::update (rect_for_node (node, per_row).adjusted (-2, -2, 2, 2));
      }
      void node_state_widget::update()
      {
        QToolTip::hideText();

        QWidget::update();
      }
      void node_state_widget::update_nodes_with_state (const QString& s)
      {
        for (int i (0); i < _nodes.size(); ++i)
        {
          if (_nodes[i]._state == s)
          {
            update (i);
          }
        }
      }

      void node_state_widget::states_actions_long_text
        (const QString& action, const QString& long_text)
      {
        _long_action[action] = long_text;
      }

      QString node_state_widget::full_action_name
        (QString action, const QSet<int>& host_ids) const
      {
        fhg_assert (!host_ids.empty(), "an action needs to be executed on at least one host");

        const QString first_hostname (node (*host_ids.begin())._hostname);
        const QString replacement
          ( host_ids.size() <= 1 ? first_hostname
          : host_ids.size() == 2 ? QString ("%1 (and one other)")
                              .arg (first_hostname)
          : QString ("%1 (and %2 others)")
          .arg (first_hostname).arg (host_ids.size() - 1)
          );

        return QString ( _long_action.contains (action)
                       ? _long_action[action]
                       : action
                       ).replace ("{hostname}", replacement);
      }

      void node_state_widget::states_actions_requires_confirmation
        (const QString& action, bool requires)
      {
        if (requires)
        {
          _action_requires_confirmation.insert (action);
        }
        else
        {
          _action_requires_confirmation.remove (action);
        }
      }
      void node_state_widget::states_actions_arguments
        (const QString& action, const QList<monitor_client::action_argument_data>& arguments)
      {
        _action_arguments[action] = arguments;
      }
      void node_state_widget::states_actions_expected_next_state
        (const QString& action, const QString& expected_next_state)
      {
        _action_expects_next_state[action] = expected_next_state;
      }

      void node_state_widget::nodes_details
        (const QString& hostname, const boost::optional<QString>& details)
      {
        const boost::optional<size_t> node_index (node_index_by_name (hostname));
        if (node_index)
        {
          node (*node_index)._details = details;
        }
      }

      void node_state_widget::nodes_state
        (const QString& hostname, const boost::optional<QString>& state)
      {
        _pending_updates.remove (hostname);
        _nodes_to_update << hostname;

        if (_ignore_next_nodes_state.contains (hostname))
        {
          _ignore_next_nodes_state.remove (hostname);
          return;
        }

        const boost::optional<size_t> node_index (node_index_by_name (hostname));
        if (node_index)
        {
          node_type& n (node (*node_index));

          const boost::optional<QString> old_state (n._state);
          const boost::optional<QString> old_expected_state
            (n._expects_state_change);
          n._state = state;
          n._state_update_time = QDateTime::currentDateTime();
          n._expects_state_change = boost::none;

          if (old_state != state || old_expected_state)
          {
            update (*node_index);

            if (n._watched)
            {
              _log->warning ( QString ("%3: State changed from %1 to %2.")
                            .arg (old_state.get_value_or ("unknown"))
                            .arg (state.get_value_or ("unknown"))
                            .arg (hostname)
                            );
            }
          }
        }
      }

      void node_state_widget::nodes (QStringList hostnames)
      {
        const int old_height (heightForWidth (width()));

        QMutableListIterator<node_type> i (_nodes);
        int index (0);

        QStringList update_requests;
        while (i.hasNext())
        {
          node_type& node (i.next());
          const QString& hostname (node._hostname);
          if (!hostnames.contains (hostname))
          {
            node._state = boost::none;
            node._state_update_time = QDateTime::currentDateTime();
            node._expects_state_change = boost::none;

            _pending_updates.remove (hostname);
            _nodes_to_update.remove (hostname);

            remove_from_selection (index);
          }
          else
          {
            if (node._state == boost::none)
            {
              update_requests << hostname;
            }
            hostnames.removeOne (hostname);
          }
          ++index;
        }

        foreach (const QString& hostname, hostnames)
        {
          _nodes << node_type (hostname);
          update (_nodes.size() - 1);
          update_requests << hostname;
        }

        rebuild_node_index();

        foreach (const QString& hostname, update_requests)
        {
          if ( !_pending_updates.contains (hostname)
             && !_nodes_to_update.contains (hostname)
             )
          {
            _nodes_to_update << hostname;
          }
        }

        if (old_height != heightForWidth (width()))
        {
          updateGeometry();
        }

        setWindowTitle ( tr ("%1; watching %2 nodes")
                       .arg (_window_title)
                       .arg (_nodes.size())
                       );
      }

      void node_state_widget::action_result
        ( const QString& host
        , const QString& action
        , const monitor_client::action_result_code& result
        , const boost::optional<QString>& message
        , QMap<QString, QString> additional_data
        )
      {
        if (message)
        {
          const QString msg (QString ("%1: %2").arg (host).arg (*message));

          switch (result)
          {
          case monitor_client::okay:
            _log->information (msg);
            break;

          case monitor_client::fail:
            _log->critical (msg);
            break;

          case monitor_client::warn:
            _log->warning (msg);
            break;
          }
        }
      }

      void node_state_widget::refresh_stati()
      {
        _monitor_client->request_status (_nodes_to_update);
        _pending_updates.unite (_nodes_to_update);
        _nodes_to_update.clear();
      }

      boost::optional<int> node_state_widget::node_at (const QPoint& pos) const
      {
        const int x (pos.x());
        const int y (pos.y());

        if ( x < base_coord || y < base_coord
           || x > (base_coord + items_per_row (width()) * per_step)
           )
        {
          return boost::none;
        }

        const int row ((y - base_coord + per_step - 1) / per_step - 1);
        const int column ((x - base_coord + per_step - 1) / per_step - 1);
        const int i (row * items_per_row (width()) + column);

        return boost::make_optional
          ( i < node_count()
          && (column * per_step + base_coord + item_size) >= x
          && (row * per_step + base_coord + item_size) >= y
          , i
          );
      }

      const state_description&
        node_state_widget::state (const boost::optional<QString>& name) const
      {
        return _legend_widget->state (name);
      }

      boost::optional<size_t>
        node_state_widget::node_index_by_name (const QString& hostname) const
      {
        const QMap<QString, size_t>::const_iterator it
          (_node_index_by_hostname.find (hostname));
        return it == _node_index_by_hostname.end()
          ? boost::none : boost::optional<size_t> (*it);
      }

      void node_state_widget::rebuild_node_index()
      {
        _node_index_by_hostname.clear();
        for (int i (0); i < _nodes.size(); ++i)
        {
          _node_index_by_hostname[node (i)._hostname] = i;
        }
      }

      const node_state_widget::node_type& node_state_widget::node (int index) const
      {
        return _nodes.at (index);
      }
      node_state_widget::node_type& node_state_widget::node (int index)
      {
        return _nodes[index];
      }
      int node_state_widget::node_count() const
      {
        return _nodes.size();
      }

      void node_state_widget::paintEvent (QPaintEvent* event)
      {
        QPainter painter (this);
        painter.setRenderHint (QPainter::Antialiasing);

        const int per_row (items_per_row (width()));

        const int column_begin
          ( event->rect().left() < base_coord
          ? 0
          : ((event->rect().left() - base_coord + per_step - 1) / per_step - 1)
          );
        const int column_end
          ( event->rect().right() > per_row * per_step + base_coord + item_size
          ? per_row + 1
          : ((event->rect().right() - base_coord + per_step - 1) / per_step - 1) + 1
          );

        const int row_begin
          ( event->rect().top() < base_coord
          ? 0
          : ((event->rect().top() - base_coord + per_step - 1) / per_step - 1)
          );
        const int row_end
          ( event->rect().bottom() > ((node_count() + per_row - 1) / per_row) * per_step + base_coord + item_size
          ? ((node_count() + per_row - 1) / per_row) + 1
          : ((event->rect().bottom() - base_coord + per_step - 1) / per_step - 1) + 2
          );

        for (int row (row_begin); row < row_end; ++row)
        {
          const int row_base (row * per_row);
          for (int column (column_begin); column < column_end; ++column)
          {
            const int i (row_base + column);
            if (i < node_count())
            {
              painter.drawPixmap ( rect_for_node (i, per_row)
                                 , state (node (i)._state)._pixmap
                                 );

              if (_selection.contains (i))
              {
                painter.setBrush (Qt::Dense3Pattern);
                painter.drawRect (rect_for_node (i, per_row));
              }

              if (node (i)._expects_state_change)
              {
                const QRectF rect (rect_for_node (i, per_row));
                const QPointF points[3] =
                  {rect.bottomRight(), rect.bottomLeft(), rect.topRight()};
                painter.setBrush (state (*node (i)._expects_state_change)._brush);
                painter.drawPolygon (points, sizeof (points) / sizeof (*points));
              }

              if (node (i)._watched)
              {
                painter.setBrush (Qt::Dense6Pattern);
                painter.drawRect (rect_for_node (i, per_row));
              }
            }
            else
            {
              break;
            }
          }
        }
      }

      void node_state_widget::clear_selection()
      {
        foreach (const int index, _selection)
        {
          update (index);
        }
        _selection.clear();
      }

      void node_state_widget::add_to_selection (const int& node)
      {
        _selection.append (node);
        update (node);
      }

      void node_state_widget::remove_from_selection (const int& node)
      {
        _selection.removeOne (node);
        update (node);
      }

      void node_state_widget::mouseReleaseEvent (QMouseEvent* event)
      {
        if (event->button() == Qt::LeftButton && event->buttons() == Qt::NoButton)
        {
          const boost::optional<int> node (node_at (event->pos()));

          if (node)
          {
            if (event->modifiers() == Qt::NoModifier)
            {
              clear_selection();
              add_to_selection (*node);
              _last_manual_selection = *node;
            }
            else if (event->modifiers() == Qt::ControlModifier)
            {
              if (_selection.contains (*node))
              {
                remove_from_selection (*node);
                _last_manual_selection = boost::none;
              }
              else
              {
                add_to_selection (*node);
                _last_manual_selection = *node;
              }
            }
            else if (event->modifiers() & Qt::ShiftModifier)
            {
              if (!(event->modifiers() & Qt::ControlModifier))
              {
                clear_selection();
                if (_last_manual_selection)
                {
                  add_to_selection (*_last_manual_selection);
                }
              }
              if (_last_manual_selection)
              {
                for ( int i (qMin (*_last_manual_selection, *node) + 1)
                        ; i < qMax (*_last_manual_selection, *node)
                        ; ++i
                    )
                {
                  add_to_selection (i);
                }
              }
              else
              {
                _last_manual_selection = *node;
              }
              add_to_selection (*node);
            }
            else
            {
              QWidget::mouseReleaseEvent (event);
            }
          }
          else
          {
            if (event->modifiers() == Qt::NoModifier)
            {
              clear_selection();
              _last_manual_selection = boost::none;
            }
            else
            {
              QWidget::mouseReleaseEvent (event);
            }
          }
        }
        else
        {
          QWidget::mouseReleaseEvent (event);
        }
      }

      namespace
      {
        int string_to_int (const QString& str)
        {
          const std::string stdstr (str.toStdString());
          fhg::util::parse::position_string pos (stdstr);
          return fhg::util::read_int (pos);
        }

        QString checkbox_to_string (const QCheckBox* box)
        {
          return box->isChecked() ? "true" : "false";
        }
        QString spinbox_to_string (const QSpinBox* box)
        {
          return QString ("%1").arg (box->value());
        }

        std::pair<QWidget*, boost::function<QString()> > widget_for_item
          (const monitor_client::action_argument_data& item)
        {
          switch (*item._type)
          {
          case monitor_client::action_argument_data::boolean:
            {
              QCheckBox* box (new QCheckBox);
              box->setChecked ( fhg::util::read_bool
                              (item._default.get_value_or ("false").toStdString())
                              );
              return std::pair<QWidget*, boost::function<QString()> >
                (box, boost::bind (checkbox_to_string, box));
            }

          case monitor_client::action_argument_data::directory:
            {
              fhg::util::qt::file_line_edit* edit
                ( new fhg::util::qt::file_line_edit
                (QFileDialog::Directory, item._default.get_value_or (""))
                );
              return std::pair<QWidget*, boost::function<QString()> >
                (edit, boost::bind (&fhg::util::qt::file_line_edit::text, edit));
            }

          case monitor_client::action_argument_data::duration:
            {
              QSpinBox* edit (new QSpinBox);
              edit->setMinimum (1);
              edit->setMaximum (INT_MAX);
              edit->setSuffix (" h");
              edit->setValue (string_to_int (item._default.get_value_or ("1")));
              return std::pair<QWidget*, boost::function<QString()> >
                (edit, boost::bind (spinbox_to_string, edit));
            }

          case monitor_client::action_argument_data::filename:
            {
              fhg::util::qt::file_line_edit* edit
                ( new fhg::util::qt::file_line_edit
                (QFileDialog::AnyFile, item._default.get_value_or (""))
                );
              return std::pair<QWidget*, boost::function<QString()> >
                (edit, boost::bind (&fhg::util::qt::file_line_edit::text, edit));
            }

          case monitor_client::action_argument_data::integer:
            {
              QSpinBox* edit (new QSpinBox);
              edit->setMinimum (INT_MIN);
              edit->setMaximum (INT_MAX);
              edit->setValue (string_to_int (item._default.get_value_or ("0")));
              return std::pair<QWidget*, boost::function<QString()> >
                (edit, boost::bind (spinbox_to_string, edit));
            }

          case monitor_client::action_argument_data::string:
            {
              QLineEdit* edit (new QLineEdit (item._default.get_value_or ("")));
              return std::pair<QWidget*, boost::function<QString()> >
                (edit, boost::bind (&QLineEdit::text, edit));
            }
          default:
            abort ();
          }
        }
      }

      namespace
      {
        QString possibly_elide (QString in, int max, QString suffix)
        {
          return in.size() <= max ? in : in.left (max) + ".." + suffix;
        }
      }

      void node_state_widget::trigger_action
        (const QStringList& hosts, const QSet<int>& host_ids, const QString& action)
      {
        QMap<QString, boost::function<QString()> > value_getters;

        if ( _action_arguments.contains (action)
           && !_action_arguments[action].isEmpty()
           )
        {
          QDialog* dialog (new QDialog);
          dialog->setWindowTitle
            (tr ("Parameters for %1").arg (action));
          new QVBoxLayout (dialog);

          QWidget* wid (new QWidget (dialog));
          QFormLayout* layout (new QFormLayout (wid));

          foreach (const monitor_client::action_argument_data& item, _action_arguments[action])
          {
            if (!item._type)
            {
              throw std::runtime_error ("action argument without type");
            }

            const std::pair<QWidget*, boost::function<QString()> > ret
              (widget_for_item (item));
            layout->addRow (item._label.get_value_or (item._name), ret.first);
            value_getters[item._name] = ret.second;
          }

          QDialogButtonBox* buttons
            ( new QDialogButtonBox ( QDialogButtonBox::Abort | QDialogButtonBox::Ok
                                   , Qt::Horizontal
                                   , dialog
                                   )
            );

          dialog->connect (buttons, SIGNAL (accepted()), SLOT (accept()));
          dialog->connect (buttons, SIGNAL (rejected()), SLOT (reject()));

          dialog->layout()->addWidget (wid);
          dialog->layout()->addWidget (buttons);

          if (!dialog->exec())
          {
            return;
          }
        }

        if (_action_requires_confirmation.contains (action))
        {
          const QString action_name (full_action_name (action, host_ids));

          const QString hostnames
            ( hosts.size() == 1 ? "on host " + hosts.first()
            : possibly_elide ( "on hosts " + hosts.join (", ")
                             , 200 //! \todo Depending on number of hosts?
                             , " (and more)"
                             )
            );

          QString params;

          if (!value_getters.isEmpty())
          {
            params += "<li>with arguments<ul>";

            for ( QMap<QString, boost::function<QString()> >::const_iterator i
                   (value_getters.constBegin())
                ; i != value_getters.constEnd()
                ; ++i
                )
            {
              params += QString ("<li><b>%1</b> = %2").arg (i.key()).arg (i.value()());
            }
            params += "</ul><br>";
          }

          if ( QMessageBox::question
               ( this
               , tr ("Execute \"%1\"?").arg (action_name)
               , tr ("You will execute<ul><li>%1<li>%2%3</ul>Continue?")
               . arg (action_name).arg (hostnames).arg (params)
               , QMessageBox::Yes | QMessageBox::No
               , QMessageBox::No
               )
             == QMessageBox::No
             )
          {
            return;
          }
        }

        foreach (QString const &host, hosts)
        {
          _monitor_client->request_action (host, action, value_getters);
        }

        if (_action_expects_next_state.contains (action))
        {
          const QString expected (_action_expects_next_state[action]);
          const QSet<QString> currently_pending_hosts
            (hosts.toSet().intersect (_pending_updates));
          _ignore_next_nodes_state |= currently_pending_hosts;

          BOOST_FOREACH (const QString& host, hosts)
          {
            const size_t index (*node_index_by_name (host));
            node (index)._expects_state_change = expected;
            update (index);
          }
        }
      }

      bool node_state_widget::event (QEvent* event)
      {
        switch (event->type())
        {
        case QEvent::ToolTip:
          {
            QHelpEvent* help_event (static_cast<QHelpEvent*> (event));

            const boost::optional<int> node_index (node_at (help_event->pos()));
            if (node_index)
            {
              const boost::optional<QString> s (node (*node_index)._state);
              const QString displayed_state
                ( s
                ? state (s)._descriptive_name.get_value_or (*s)
                : "unknown state"
                );
              QToolTip::showText
                ( help_event->globalPos()
                , QString ("%1: %2%3 [last update: %4]")
                .arg (node (*node_index)._hostname)
                .arg (displayed_state)
                .arg ( node (*node_index)._details
                     ? QString (*node (*node_index)._details).prepend (" (").append (")")
                     : ""
                     )
                .arg (node (*node_index)._state_update_time.toString())
                );
              event->accept();
              return true;
            }

            QToolTip::hideText();
            event->ignore();
            return false;
          }
        case QEvent::ContextMenu:
          {
            //! \todo Block updates while menu is open or close menu when
            //! state of node changes.
            QContextMenuEvent* context_menu_event (static_cast<QContextMenuEvent*> (event));

            const boost::optional<int> node_index (node_at (context_menu_event->pos()));

            if (!node_index || !_selection.contains (*node_index))
            {
              clear_selection();
            }

            if (node_index)
            {
              _monitor_client->pause();

              QSet<int> nodes (QSet<int>::fromList (_selection));
              nodes << *node_index;

              QStringList hostnames;

              QSet<QString> action_name_intersection
                (QSet<QString>::fromList (state (node (*node_index)._state)._actions));
              QSet<QString> action_name_union;

              bool one_node_expects_state_change (false);

              foreach (const int index, nodes)
              {
                const node_type& n (node (index));
                hostnames << n._hostname;

                one_node_expects_state_change
                  = one_node_expects_state_change || n._expects_state_change;

                const QSet<QString> actions
                  (QSet<QString>::fromList (state (n._state)._actions));

                action_name_union.unite (actions);
                action_name_intersection.intersect (actions);
              }

              QMenu context_menu;

              QSet<QString> triggering_actions;
              QSet<QString> non_triggering_actions;

              foreach (const QString& action_name, action_name_union)
              {
                if (_action_expects_next_state.contains (action_name))
                {
                  triggering_actions << action_name;
                }
                else
                {
                  non_triggering_actions << action_name;
                }
              }

              foreach (const QString& action_name, triggering_actions)
              {
                QAction* action
                  (context_menu.addAction (full_action_name (action_name, nodes)));

                if ( !one_node_expects_state_change
                   && action_name_intersection.contains (action_name)
                   )
                {
                  fhg::util::qt::boost_connect<void (void)>
                    ( action
                    , SIGNAL (triggered())
                    , this
                    , boost::bind ( &node_state_widget::trigger_action
                                  , this
                                  , hostnames
                                  , nodes
                                  , action_name
                                  )
                    );
                }
                else
                {
                  action->setEnabled (false);
                }
              }

              if (!action_name_union.empty())
              {
                context_menu.addSeparator();
              }

              foreach (const QString& action_name, non_triggering_actions)
              {
                QAction* action
                  (context_menu.addAction (full_action_name (action_name, nodes)));

                if ( !one_node_expects_state_change
                   && action_name_intersection.contains (action_name)
                   )
                {
                  fhg::util::qt::boost_connect<void (void)>
                    ( action
                    , SIGNAL (triggered())
                    , this
                    , boost::bind ( &node_state_widget::trigger_action
                                  , this
                                  , hostnames
                                  , nodes
                                  , action_name
                                  )
                    );
                }
                else
                {
                  action->setEnabled (false);
                }
              }

              if (!action_name_union.empty())
              {
                context_menu.addSeparator();
              }

              {
                bool all (true);
                bool none (true);

                foreach (const int index, nodes)
                {
                  all = all && node (index)._watched;
                  none = none && !node (index)._watched;
                }

                const QString text (tr ("Notify on state changes"));

                if (all || none)
                {
                  QAction* action (context_menu.addAction (text));
                  action->setCheckable (true);
                  action->setChecked (all);
                  foreach (const int index, nodes)
                  {
                    fhg::util::qt::boost_connect<void (bool)>
                      ( action
                      , SIGNAL (toggled (bool))
                      , _monitor_client
                      , boost::bind (&node_type::watched, &(node (index)), !all)
                      );

                    fhg::util::qt::boost_connect<void (void)>
                      ( action
                      , SIGNAL (triggered())
                      , this
                      , boost::bind (&node_state_widget::update, this, index)
                      );
                  }
                }
                else
                {
                  QMenu* menu (context_menu.addMenu (text));

                  QAction* watch_all (menu->addAction (tr ("All")));
                  QAction* unwatch_all (menu->addAction (tr ("None")));

                  foreach (const int index, nodes)
                  {
                    fhg::util::qt::boost_connect<void (void)>
                      ( unwatch_all
                      , SIGNAL (triggered())
                      , _monitor_client
                      , boost::bind (&node_type::watched, &(node (index)), false)
                      );
                    fhg::util::qt::boost_connect<void (void)>
                      ( watch_all
                      , SIGNAL (triggered())
                      , _monitor_client
                      , boost::bind (&node_type::watched, &(node (index)), true)
                      );

                    fhg::util::qt::boost_connect<void (void)>
                      ( unwatch_all
                      , SIGNAL (triggered())
                      , this
                      , boost::bind (&node_state_widget::update, this, index)
                      );
                    fhg::util::qt::boost_connect<void (void)>
                      ( watch_all
                      , SIGNAL (triggered())
                      , this
                      , boost::bind (&node_state_widget::update, this, index)
                      );
                  }
                }
              }

              context_menu.exec (context_menu_event->globalPos());
              _monitor_client->resume();
              event->accept();
              return true;
            }

            event->ignore();
            return false;
          }
        default:
          return QWidget::event (event);
        }
      }

      int node_state_widget::heightForWidth (int width) const
      {
        const int per_row (items_per_row (width));
        return (node_count() + per_row - 1) / per_row * per_step;
      }

      void node_state_widget::sort_by
        (boost::function<bool (const node_type&, const node_type&)> pred)
      {
        _monitor_client->pause();

        QList<QString> selected_hostnames;

        foreach (const int& index, _selection)
        {
          selected_hostnames << node (index)._hostname;
        }

        qStableSort (_nodes.begin(), _nodes.end(), pred);

        rebuild_node_index();

        _last_manual_selection = boost::none;

        _selection.clear();
        BOOST_FOREACH (const QString& name, selected_hostnames)
        {
          _selection << *node_index_by_name (name);
        }

        update();

        _monitor_client->resume();
      }

      void node_state_widget::sort_by_name()
      {
        // [](const QString& l, const QString& r) -> bool
        // {
        //   return fhg::util::alphanum::less() (l.toStdString(), r.toStdString());
        // }

        fhg::util::alphanum::less less;

        sort_by ( boost::bind ( &fhg::util::alphanum::less::operator(), less
                              , boost::bind ( &QString::toStdString
                                            , boost::bind (&node_type::_hostname, _1)
                                            )
                              , boost::bind ( &QString::toStdString
                                            , boost::bind (&node_type::_hostname, _2)
                                            )
                              )
                );
      }

      void node_state_widget::sort_by_state()
      {
        sort_by ( boost::bind (&node_type::_state, _1)
                < boost::bind (&node_type::_state, _2)
                );
      }

      void node_state_widget::select_all()
      {
        _selection.clear();

        for (int i (0); i < _nodes.size(); ++i)
        {
          _selection << i;
        }

        update();
      }
    }
  }
}
