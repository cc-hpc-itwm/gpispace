// bernd.loerwald@itwm.fraunhofer.de

#include "node_state_widget.hpp"

#include "parse.hpp"

#include <util/qt/boost_connect.hpp>

#include <QApplication>
#include <QCheckBox>
#include <QDebug>
#include <QGroupBox>
#include <QMenu>
#include <QPushButton>
#include <QPaintEvent>
#include <QPainter>
#include <QLabel>
#include <QPen>
#include <QScrollArea>
#include <QScrollBar>
#include <QSplitter>
#include <QStyle>
#include <QListWidgetItem>
#include <QToolTip>
#include <QVBoxLayout>
#include <QMessageBox>

#include <boost/bind.hpp>
#include <boost/optional.hpp>

#include <fhg/util/parse/error.hpp>

#include <iostream>

namespace prefix
{
  namespace
  {
    const qreal item_size (20.0);
    const qreal pen_size (0.0);
    const qreal padding (0.0);

    const qreal per_step (item_size + padding);
    const qreal node_size (item_size - pen_size);

    const qreal base_coord (padding + pen_size / 2.0);

    int items_per_row (int width)
    {
      return (qMax (width - 2 * padding, per_step)) / per_step;
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

  communication::communication (const QString& host, int port, QObject* parent)
    : QObject (parent)
    , _connection (new async_tcp_communication (host, port, this))
    , _timer (NULL)
  {
    resume();
    _connection->push ("possible_status");
  }

  void communication::request_hostlist()
  {
    _connection->push ("hosts");
  }

  void communication::pause()
  {
    delete _timer;
  }

  void communication::resume()
  {
    _timer = timer (this, 100, SLOT (check_for_incoming_messages()));
  }

  legend::legend (QWidget* parent)
    : QWidget (parent)
  {
    new QVBoxLayout (this);

    connect ( this
            , SIGNAL (state_pixmap_changed (const QString&))
            , SLOT (update (const QString&))
            );
  }

  log_widget::log_widget (QWidget* parent)
    : QListWidget (parent)
  { }

  void log_widget::information (const QString& message)
  {
    new QListWidgetItem
      ( QApplication::style()->standardIcon (QStyle::SP_MessageBoxInformation)
      , message
      , this
      );
  }

  void log_widget::warning (const QString& message)
  {
    new QListWidgetItem
      ( QApplication::style()->standardIcon (QStyle::SP_MessageBoxWarning)
      , message
      , this
      );
  }

  void log_widget::critical (const QString& message)
  {
    new QListWidgetItem
      ( QApplication::style()->standardIcon (QStyle::SP_MessageBoxCritical)
      , message
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

  node_state_widget::node_state_widget ( const QString& host
                                       , int port
                                       , legend* legend_widget
                                       , log_widget* log
                                       , QWidget* parent
                                       )
      : QWidget (parent)
      , _legend_widget (legend_widget)
      , _log (log)
      , _communication (new communication (host, port, this))
  {
    timer
      (this, 1000, boost::bind (&communication::request_hostlist, _communication));
    timer (this, 200, SLOT (refresh_stati()));

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

    connect ( _communication
            , SIGNAL (nodes (QStringList))
            , SLOT (nodes (QStringList)));
    connect ( _communication
            , SIGNAL (nodes_details (const QString&, const QString&))
            , SLOT (nodes_details (const QString&, const QString&)));
    connect ( _communication
            , SIGNAL (nodes_state (const QString&, const QString&))
            , SLOT (nodes_state (const QString&, const QString&)));
    connect ( _communication
            , SIGNAL (states_actions_long_text (const QString&, const QString&))
            , SLOT (states_actions_long_text (const QString&, const QString&)));

    connect ( _communication
            , SIGNAL (states_add (const QString&, const QStringList&))
            , _legend_widget
            , SLOT (states_add (const QString&, const QStringList&)));
    connect ( _communication
            , SIGNAL (states_layout_hint_border (const QString&, const QColor&))
            , _legend_widget
            , SLOT (states_layout_hint_border (const QString&, const QColor&)));
    connect ( _communication
            , SIGNAL (states_layout_hint_character (const QString&, const char&))
            , _legend_widget
            , SLOT (states_layout_hint_character (const QString&, const char&)));
    connect ( _communication
            , SIGNAL (states_layout_hint_color (const QString&, const QColor&))
            , _legend_widget
            , SLOT (states_layout_hint_color (const QString&, const QColor&)));
    connect ( _communication
            , SIGNAL (action_result (const QString&, const QString&, const result_code&, const boost::optional<QString>&))
            , SLOT (action_result (const QString&, const QString&, const result_code&, const boost::optional<QString>&))
            );

    connect ( _legend_widget
            , SIGNAL (state_pixmap_changed (const QString&))
            , SLOT (update_nodes_with_state (const QString&))
            );
  }

  QRect rect_for_node (const int node, const int per_row)
  {
    return QRect ( (node % per_row) * per_step + base_coord
                 , (node / per_row) * per_step + base_coord
                 , node_size
                 , node_size
                 );
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

  void legend::states_add
    (const QString& state, const QStringList& actions)
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

  void node_state_widget::states_actions_long_text
    (const QString& action, const QString& long_text)
  {
    _long_action[action] = long_text;
  }

  void node_state_widget::update_nodes_with_state (const QString& s)
  {
    for (int i (0); i < _nodes.size(); ++i)
    {
      if (_nodes[i].state() == s)
      {
        update (i);
      }
    }
  }

  void legend::update (const QString& s)
  {
    delete _state_legend[s];
    _state_legend[s] = new legend_entry (s, state (s), this);

    layout()->addWidget (_state_legend[s]);
  }

  void node_state_widget::nodes_details
    (const QString& hostname, const QString& details)
  {
    const QList<node_type>::iterator it
      ( std::find_if ( _nodes.begin()
                     , _nodes.end()
                     , boost::bind (&node_type::hostname, _1) == hostname
                     )
      );

    if (it != _nodes.end())
    {
      it->details (details);
    }
  }

  void node_state_widget::nodes_state
    (const QString& hostname, const QString& state)
  {
    const QList<node_type>::iterator it
      ( std::find_if ( _nodes.begin()
                     , _nodes.end()
                     , boost::bind (&node_type::hostname, _1) == hostname
                     )
      );

    if (it != _nodes.end())
    {
      _pending_updates.removeAll (hostname);
      _nodes_to_update << hostname;

      const boost::optional<QString> old_state (it->state());
      it->state (state);

      if (old_state != state)
      {
        update (it - _nodes.begin());

        if (it->watched())
        {
          _log->warning ( QString ("%3: State changed from %1 to %2.")
                        .arg (old_state.get_value_or ("unknown"))
                        .arg (state)
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
      const QString& hostname (node.hostname());
      if (!hostnames.contains (hostname))
      {
        node.state (boost::none);

        _pending_updates.removeAll (hostname);
        _nodes_to_update.removeAll (hostname);

        remove_from_selection (index);
      }
      else
      {
        if (node.state() == boost::none)
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
  }

  void node_state_widget::refresh_stati()
  {
    _communication->request_status (_nodes_to_update);
    _pending_updates << _nodes_to_update;
    _nodes_to_update.clear();
  }

  void communication::request_status (QStringList nodes_to_update)
  {
    static const int chunk_size (1000);

    while (!nodes_to_update.empty())
    {
      QString message ("status: [");
      while (message.size() < chunk_size && !nodes_to_update.empty())
      {
        message.append ("\"")
               .append (nodes_to_update.takeFirst())
               .append ("\",");
      }
      message.append ("]");
      _connection->push (message);
    }
  }

  void communication::request_layout_hint (const QString& state)
  {
    _connection->push (QString ("layout_hint: [\"%1\"]").arg (state));
  }

  void communication::request_action_description (const QStringList& actions)
  {
    if (!actions.empty())
    {
      QString message ("describe_action: [");
      foreach (const QString& action, actions)
      {
        message.append ("\"")
               .append (action)
               .append ("\", ");
      }
      message.append ("]");
      _connection->push (message);
    }
  }

  void communication::request_action
    (const QString& hostname, const QString& action)
  {
    _connection->push ( QString ("action: [\"%1\": \"%2\"]")
                      .arg (hostname)
                      .arg (action)
                      );
  }

  void communication::possible_status (fhg::util::parse::position& pos)
  {
    const QString state (require::label (pos));

    QStringList actions;
    require::list ( pos
                  , boost::bind ( &QStringList::push_back
                                , &actions
                                , boost::bind (require::qstring, _1)
                                )
                  );

    emit states_add (state, actions);

    request_layout_hint (state);
    request_action_description (actions);
  }

  void communication::action_description
    (fhg::util::parse::position& pos, const QString& action)
  {
    pos.skip_spaces();

    if (pos.end() || (*pos != 'l'))
    {
      throw fhg::util::parse::error::expected ("long_text", pos);
    }

    switch (*pos)
    {
    case 'l':
      ++pos;
      pos.require ("ong_text");
      require::token (pos, ":");

      emit states_actions_long_text (action, require::qstring (pos));

      break;
    }
  }

  void communication::layout_hint
    (fhg::util::parse::position& pos, const QString& state)
  {
    pos.skip_spaces();

    if (pos.end() || (*pos != 'b' && *pos != 'c'))
    {
      throw fhg::util::parse::error::expected
        ("border' or 'character' or 'color", pos);
    }

    switch (*pos)
    {
    case 'b':
      ++pos;
      pos.require ("order");
      require::token (pos, ":");

      emit states_layout_hint_border (state, require::qcolor (pos));

      break;

    case 'c':
      ++pos;
      {
        if (pos.end() || (*pos != 'h' && *pos != 'o'))
        {
          throw fhg::util::parse::error::expected ("haracter' or 'olor", pos);
        }

        switch (*pos)
        {
        case 'h':
          ++pos;
          pos.require ("aracter");
          require::token (pos, ":");

          emit states_layout_hint_character (state, pos.character());

          break;

        case 'o':
          ++pos;
          pos.require ("lor");
          require::token (pos, ":");

          emit states_layout_hint_color (state, require::qcolor (pos));

          break;
        }
      }
      break;
    }
  }
  void communication::status_update
    (fhg::util::parse::position& pos, const QString& hostname)
  {
    pos.skip_spaces();

    if (pos.end() || (*pos != 'd' && *pos != 's'))
    {
      throw fhg::util::parse::error::expected ("details' or 'state", pos);
    }

    switch (*pos)
    {
    case 'd':
      ++pos;
      pos.require ("etails");
      require::token (pos, ":");

      emit nodes_details (hostname, require::qstring (pos));

      break;

    case 's':
      ++pos;
      pos.require ("tate");
      require::token (pos, ":");

      emit nodes_state (hostname, require::qstring (pos));

      break;
    }
  }

  namespace
  {
    struct action_result_data
    {
      action_result_data()
        : _result (boost::none)
        , _message (boost::none)
      { }

      boost::optional<result_code> _result;
      boost::optional<QString> _message;

      void append (fhg::util::parse::position& pos)
      {
        pos.skip_spaces();

        if (pos.end() || (*pos != 'm' && *pos != 'r'))
        {
          throw fhg::util::parse::error::expected ("message' or 'result", pos);
        }

        switch (*pos)
        {
        case 'm':
          ++pos;
          pos.require ("essage");
          require::token (pos, ":");

          _message = require::qstring (pos);

          break;

        case 'r':
          ++pos;
          pos.require ("esult");
          require::token (pos, ":");

          {
            pos.skip_spaces();

            if (pos.end() || (*pos != 'f' && *pos != 'o' && *pos != 'w'))
            {
              throw fhg::util::parse::error::expected
                ("fail' or 'okay' or 'warn", pos);
            }

            switch (*pos)
            {
            case 'f':
              ++pos;
              pos.require ("ail");

              _result = fail;

              break;

            case 'o':
              ++pos;
              pos.require ("kay");

              _result = okay;

              break;

            case 'w':
              ++pos;
              pos.require ("arn");

              _result = warn;

              break;
            }
          }

          break;
        }
      }
    };
  }

  void communication::action_result (fhg::util::parse::position& pos)
  {
    require::token (pos, "(");
    const QString host (require::qstring (pos));
    require::token (pos, ",");
    const QString action (require::qstring (pos));
    require::token (pos, ")");
    require::token (pos, ":");

    action_result_data result;
    require::list (pos, boost::bind (&action_result_data::append, &result, _1));

    if (!result._result)
    {
      throw std::runtime_error ("action result without result code");
    }

    emit action_result (host, action, *result._result, result._message);
  }

  void node_state_widget::action_result ( const QString& host
                                        , const QString& action
                                        , const result_code& result
                                        , const boost::optional<QString>& message
                                        )
  {
    if (message)
    {
      const QString msg (QString ("%1: %2").arg (host).arg (*message));

      switch (result)
      {
      case okay:
        _log->information (msg);
        break;

      case fail:
        _log->critical (msg);
        break;

      case warn:
        _log->warning (msg);
        break;
      }
    }
  }

  void communication::check_for_incoming_messages()
  {
    const async_tcp_communication::messages_type messages (_connection->get());
    foreach (const QString& message, messages)
    {
      const std::string std_message (message.toStdString());
      fhg::util::parse::position pos (std_message);

      try
      {
        pos.skip_spaces();

        if ( pos.end()
          || ( *pos != 'a' && *pos != 'h' && *pos != 'l'
            && *pos != 'p' && *pos != 's'
             )
           )
        {
          throw fhg::util::parse::error::expected ("packet", pos);
        }

        switch (*pos)
        {
        case 'a':
          ++pos;
          pos.require ("ction_");

          if (pos.end() || (*pos != 'd' && *pos != 'r'))
          {
            throw fhg::util::parse::error::expected
              ("description' or 'result", pos);
          }

          switch (*pos)
          {
          case 'd':
            ++pos;
            pos.require ("escription");
            require::token (pos, ":");

            require::list_of_named_lists
              ( pos
              , boost::bind (&communication::action_description, this, _1, _2)
              );

            break;

          case 'r':
            ++pos;
            pos.require ("esult");
            require::token (pos, ":");

            require::list
              (pos, boost::bind (&communication::action_result, this, _1));
          }
          break;

        case 'h':
          ++pos;
          pos.require ("osts");
          require::token (pos, ":");

          {
            QStringList hostnames;
            require::list ( pos
                          , boost::bind ( &QStringList::push_back
                                        , &hostnames
                                        , boost::bind (require::qstring, _1)
                                        )
                          );

            emit nodes (hostnames);
          }

          break;

        case 'l':
          ++pos;
          pos.require ("ayout_hint");
          require::token (pos, ":");

          require::list_of_named_lists
            ( pos
            , boost::bind (&communication::layout_hint, this, _1, _2)
            );

          break;

        case 'p':
          ++pos;
          pos.require ("ossible_status");
          require::token (pos, ":");

          require::list
            (pos, boost::bind (&communication::possible_status, this, _1));

          break;

        case 's':
          ++pos;
          pos.require ("tatus");
          require::token (pos, ":");

          require::list_of_named_lists
            (pos, boost::bind (&communication::status_update, this, _1, _2));

          break;
        }
      }
      catch (const std::runtime_error& ex)
      {
        //! \todo Report back to server?
        std::cerr << "PARSE ERROR: " << ex.what() << "\nmessage: " << qPrintable (message) << "\nrest: " << pos.rest() << "\n";
      }
    }
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

  legend_entry::legend_entry
    (const QString& name, const state_description& desc, QWidget* parent)
    : QWidget (parent)
  {
    QGridLayout* layout (new QGridLayout (this));
    QLabel* label (new QLabel (this));
    label->setPixmap (desc._pixmap);
    layout->addWidget (label, 0, 0);
    layout->addWidget (new QLabel (name, this), 0, 1);
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
    legend::state (const boost::optional<QString>& name) const
  {
    if (name)
    {
      return *_states.find (*name);
    }
    else
    {
      static state_description fallback (QStringList(), '?');
      return fallback;
    }
  }

  const state_description&
    node_state_widget::state (const boost::optional<QString>& name) const
  {
    return _legend_widget->state (name);
  }


  const node_type& node_state_widget::node (int index) const
  {
    return _nodes.at (index);
  }
  node_type& node_state_widget::node (int index)
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
                             , state (node (i).state())._pixmap
                             );

          if (_selection.contains (i))
          {
            painter.setBrush (Qt::Dense3Pattern);
            painter.drawRect (rect_for_node (i, per_row));
          }

          if (node (i).watched())
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
            add_to_selection (*_last_manual_selection);
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
          QToolTip::showText
            ( help_event->globalPos()
            , QString ("%1: %2%3")
            .arg (node (*node_index).hostname())
            .arg (node (*node_index).state().get_value_or ("unknown state"))
            .arg ( node (*node_index).details()
                 ? QString (*node (*node_index).details()).prepend (" (").append (")")
                 : ""
                 )
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
          _communication->pause();

          QSet<int> nodes (QSet<int>::fromList (_selection));
          const QString hostname_replacement ( nodes.size() <= 1
                                             ? node (*node_index).hostname()
                                             : nodes.size() == 2
                                             ? QString ("%1 (and one other)")
                                             .arg (node (*node_index).hostname())
                                             : QString ("%1 (and %2 others)")
                                             .arg (node (*node_index).hostname())
                                             .arg (nodes.size() - 1)
                                             );
          nodes << *node_index;

          QStringList hostnames;

          QSet<QString> action_name_intersection
            (QSet<QString>::fromList (state (node (*node_index).state())._actions));
          QSet<QString> action_name_union;

          foreach (const int index, nodes)
          {
            const node_type& n (node (index));
            hostnames << n.hostname();

            const QSet<QString> actions
              (QSet<QString>::fromList (state (n.state())._actions));

            action_name_union.unite (actions);
            action_name_intersection.intersect (actions);
          }

          QMenu context_menu;

          foreach (const QString& action_name, action_name_union)
          {
            QAction* action ( context_menu.addAction
                              ( QString ( _long_action.contains (action_name)
                                        ? _long_action[action_name]
                                        : action_name
                                        )
                              .replace ("{hostname}", hostname_replacement)
                              )
                            );

            if (action_name_intersection.contains (action_name))
            {
              foreach (const QString& hostname, hostnames)
              {
                fhg::util::qt::boost_connect<void (void)>
                  ( action
                  , SIGNAL (triggered())
                  , _communication
                  , boost::bind ( &communication::request_action
                                , _communication
                                , hostname
                                , action_name
                                )
                  );
              }
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
              all = all && node (index).watched();
              none = none && !node (index).watched();
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
                  , _communication
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
                  , _communication
                  , boost::bind (&node_type::watched, &(node (index)), false)
                  );
                fhg::util::qt::boost_connect<void (void)>
                  ( watch_all
                  , SIGNAL (triggered())
                  , _communication
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
          _communication->resume();
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

  void node_state_widget::sort_by_name()
  {
    _communication->pause();

    QList<QString> selected_hostnames;

    foreach (const int& index, _selection)
    {
      selected_hostnames << node (index).hostname();
    }

    fhg::util::alphanum::less less;

    qStableSort
      ( _nodes.begin(), _nodes.end()
      // , [](const QString& l, const QString& r) -> bool
      // {
      //   return fhg::util::alphanum::less() (l.toStdString(), r.toStdString());
      // }
      , boost::bind ( &fhg::util::alphanum::less::operator(), less
                    , boost::bind ( &QString::toStdString
                                  , boost::bind (&node_type::hostname, _1)
                                  )
                    , boost::bind ( &QString::toStdString
                                  , boost::bind (&node_type::hostname, _2)
                                  )
                    )
      );

    _selection.clear();
    _last_manual_selection = boost::none;

    int i (0);
    foreach (const node_type& node, _nodes)
    {
      if (selected_hostnames.contains (node.hostname()))
      {
        _selection << i;
      }

      ++i;
    }

    update();
    _communication->resume();
  }
  void node_state_widget::sort_by_state()
  {
    _communication->pause();

    QList<QString> selected_hostnames;

    foreach (const int& index, _selection)
    {
      selected_hostnames << node (index).hostname();
    }

    qStableSort ( _nodes.begin(), _nodes.end()
                , boost::bind (&node_type::state, _1)
                < boost::bind (&node_type::state, _2)
                );

    _selection.clear();
    _last_manual_selection = boost::none;

    int i (0);
    foreach (const node_type& node, _nodes)
    {
      if (selected_hostnames.contains (node.hostname()))
      {
        _selection << i;
      }

      ++i;
    }

    update();
    _communication->resume();
  }

  void node_state_widget::select_all()
  {
    _selection.clear();

    for (int i (_nodes.size() - 1); i >= 0; --i)
    {
      _selection << i;
    }

    update();
  }
}

#include <QApplication>

int main (int argc, char** argv)
{
  QApplication app (argc, argv);

  const QString host ("localhost");
  const int port (44451);

  QSplitter window (Qt::Vertical);

  QWidget* main (new QWidget (&window));
  prefix::log_widget* log (new prefix::log_widget (&window));

  QWidget* sidebar (new QWidget (main));
  prefix::legend* legend_widget (new prefix::legend (sidebar));

  QScrollArea* content (new QScrollArea (main));
  QWidget* inner (new QWidget (content));

  prefix::node_state_widget* node_widget
    (new prefix::node_state_widget (host, port, legend_widget, log, inner));

  QGroupBox* sort_box (new QGroupBox (QObject::tr ("sort"), sidebar));

  {
    QPushButton* sort_by_state
      (new QPushButton (QObject::tr ("by state"), sort_box));
    QPushButton* sort_by_name
      (new QPushButton (QObject::tr ("by name"), sort_box));

    node_widget->connect
      (sort_by_state, SIGNAL (clicked()), SLOT (sort_by_state()));
    node_widget->connect
      (sort_by_name, SIGNAL (clicked()), SLOT (sort_by_name()));

    QVBoxLayout* layout (new QVBoxLayout (sort_box));
    layout->addWidget (sort_by_state);
    layout->addWidget (sort_by_name);
  }

  {
    QPushButton* clear_log
      (new QPushButton (QObject::tr ("clear log"), sidebar));
    log->connect (clear_log, SIGNAL (clicked()), SLOT (clear()));

    QCheckBox* follow_logging
      (new QCheckBox (QObject::tr ("follow logging"), sidebar));
    log->connect (follow_logging, SIGNAL (toggled (bool)), SLOT (follow (bool)));
    follow_logging->setChecked (true);

    QVBoxLayout* layout (new QVBoxLayout (sidebar));
    layout->addWidget (legend_widget);
    layout->addWidget (sort_box);
    layout->addStretch();
    layout->addWidget (follow_logging);
    layout->addWidget (clear_log);
  }

  {
    QVBoxLayout* layout (new QVBoxLayout (inner));
    layout->addWidget (node_widget);

    content->setWidget (inner);
    content->setWidgetResizable (true);
  }

  {
    QHBoxLayout* layout (new QHBoxLayout (main));
    layout->addWidget (content);
    layout->addWidget (sidebar);
  }

  window.show();

  return app.exec();
}
