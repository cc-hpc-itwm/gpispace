// bernd.loerwald@itwm.fraunhofer.de

#include "node_state_widget.hpp"

#include <util/qt/boost_connect.hpp>

#include <QDebug>
#include <QMenu>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QScrollArea>
#include <QScrollBar>
#include <QToolTip>
#include <QVBoxLayout>

#include <boost/bind.hpp>
#include <boost/optional.hpp>

#include <fhg/util/parse/error.hpp>
#include <fhg/util/num.hpp>

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

    void timer (QObject* parent, int timeout, const char* slot)
    {
      QTimer* timer (new QTimer (parent));
      QObject::connect (timer, SIGNAL (timeout()), parent, slot);
      timer->start (timeout);
    }
    void timer (QObject* parent, int timeout, boost::function<void()> fun)
    {
      QTimer* timer (new QTimer (parent));
      fhg::util::qt::boost_connect<void()>
        (timer, SIGNAL (timeout()), parent, fun);
      timer->start (timeout);
    }

    namespace require
    {
      void token (fhg::util::parse::position& pos, const std::string& what)
      {
        pos.skip_spaces();
        pos.require (what);
      }

      QString qstring (fhg::util::parse::position& pos)
      {
        token (pos, "\"");
        return QString::fromStdString (pos.until ('"'));
      }

      QString label (fhg::util::parse::position& pos)
      {
        const QString key (qstring (pos));
        token (pos, ":");
        return key;
      }

      QColor qcolor (fhg::util::parse::position& pos)
      {
        pos.skip_spaces();
        return QColor (fhg::util::read_uint (pos));
      }

      void list ( fhg::util::parse::position& pos
                , const boost::function<void (fhg::util::parse::position&)>& f
                )
      {
        pos.list ('[', ',', ']', f);
      }

      void named_list
        ( fhg::util::parse::position& pos
        , const boost::function<void (fhg::util::parse::position&, const QString&)>& f
        )
      {
        list (pos, boost::bind (f, _1, label (pos)));
      }

      void list_of_named_lists
        ( fhg::util::parse::position& pos
        , const boost::function<void (fhg::util::parse::position&, const QString&)>& f
        )
      {
        list (pos, boost::bind (named_list, _1, f));
      }
    }
  }

  node_state_widget::node_state_widget (QWidget* parent)
    : QWidget (parent)
    , _connection (new async_tcp_communication)
  {
    _connection->push ("possible_status_list");

    timer (this, 100, SLOT (check_for_incoming_messages()));
    timer (this, 1000, boost::bind ( &async_tcp_communication::push
                                   , _connection
                                   , QString ("host_list")
                                   )
          );
    timer (this, 200, SLOT (refresh_stati()));

    // qStableSort (_nodes.begin(), _nodes.end(), boost::bind (&node_type::less_by_hostname, _1, _2));

    setSizeIncrement (per_step, per_step);
    QSizePolicy pol (QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    pol.setHeightForWidth (true);
    setSizePolicy (pol);
  }

  QRectF rect_for_node (const int node, const int per_row)
  {
    return QRectF ( (node % per_row) * per_step + base_coord
                 , (node / per_row) * per_step + base_coord
                 , node_size
                 , node_size
                 );
  }

  void node_state_widget::update (int node)
  {
    const int per_row (items_per_row (width()));

    QWidget::update
      (rect_for_node (node, per_row).toRect().adjusted (-1, -1, 1, 1));
  }

  void node_state_widget::possible_status (fhg::util::parse::position& pos)
  {
    const QString state (require::label (pos));

    QStringList actions;
    require::list ( pos
                 , boost::bind ( &QStringList::push_back
                               , &actions
                               , boost::bind (require::qstring, _1)
                               )
                 );

    _connection->push (QString ("layout_hint %1").arg (state));
    _connection->push (QString ("describe_action %1").arg (actions.join (" ")));

    _states.insert (state, state_description (actions));
  }

  void node_state_widget::action_description
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
      _long_action[action] = require::qstring (pos);
      break;
    }
  }

  void node_state_widget::layout_hint ( fhg::util::parse::position& pos
                                      , const QString& state
                                      )
  {
    pos.skip_spaces();

    if (pos.end() || (*pos != 'b' && *pos != 'c'))
    {
      throw fhg::util::parse::error::expected
        ("border' or 'character' or 'color", pos);
    }

    state_description& desc (_states[state]);

    switch (*pos)
    {
    case 'b':
      ++pos;
      pos.require ("order");
      require::token (pos, ":");
      desc._pen = require::qcolor (pos);
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
          desc._character = pos.character();
          break;

        case 'o':
          ++pos;
          pos.require ("lor");
          require::token (pos, ":");
          desc._brush = require::qcolor (pos);
          break;
        }
      }
      break;
    }

    desc.reset();
  }

  void node_state_widget::status_update
    (fhg::util::parse::position& pos, const QString& hostname)
  {
    pos.skip_spaces();

    if (pos.end() || (*pos != 's'))
    {
      throw fhg::util::parse::error::expected ("state", pos);
    }

    _pending_updates.removeAll (hostname);
    const QVector<node_type>::iterator it
      ( std::find_if ( _nodes.begin()
                     , _nodes.end()
                     , boost::bind (&node_type::hostname_is, _1, hostname)
                     )
      );

    if (it != _nodes.end())
    {
      _nodes_to_update << hostname;
    }

    switch (*pos)
    {
    case 's':
      ++pos;
      pos.require ("tate");
      require::token (pos, ":");

      {
        const QString state (require::qstring (pos));
        if (it != _nodes.end())
        {
          it->state (state);
          update (it - _nodes.begin());
        }
      }

      break;
    }
  }

  void node_state_widget::check_for_incoming_messages()
  {
    const async_tcp_communication::messages_type messages (_connection->get());
    foreach (const QString& message, messages)
    {
      const std::string std_message (message.toStdString());
      fhg::util::parse::position pos (std_message);

      try
      {
        pos.skip_spaces();

        if (pos.end())
        {
          throw fhg::util::parse::error::expected ("packet", pos);
        }

        switch (*pos)
        {
        case 'a':
          ++pos;
          pos.require ("ction_description");
          require::token (pos, ":");

          require::list_of_named_lists
            ( pos
            , boost::bind (&node_state_widget::action_description, this, _1, _2)
            );

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

            const int old_height (heightForWidth (width()));

            QMutableVectorIterator<node_type> i (_nodes);
            while (i.hasNext())
            {
              const QString& hostname (i.next().hostname());
              int index (hostnames.indexOf (hostname));
              if (index == -1)
              {
                i.remove();
                update (index);
                update (_nodes.size());
                _pending_updates.removeAll (hostname);
                _nodes_to_update.removeAll (hostname);
              }
              else
              {
                hostnames.removeOne (hostname);
              }
            }

            foreach (const QString& hostname, hostnames)
            {
              _nodes << node_type (hostname);
              update (_nodes.size() - 1);
              _nodes_to_update << hostname;
            }

            if (old_height != heightForWidth (width()))
            {
              updateGeometry();
            }
          }

          break;

        case 'l':
          ++pos;
          pos.require ("ayout_hint");
          require::token (pos, ":");

          require::list_of_named_lists
            ( pos
            , boost::bind (&node_state_widget::layout_hint, this, _1, _2)
            );

          break;

        case 'p':
          ++pos;
          pos.require ("ossible_status");
          require::token (pos, ":");

          require::list
            (pos, boost::bind (&node_state_widget::possible_status, this, _1));

          break;

        case 's':
          ++pos;
          pos.require ("tatus");
          require::token (pos, ":");

          require::list_of_named_lists
            (pos, boost::bind (&node_state_widget::status_update, this, _1, _2));

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

  void node_state_widget::refresh_stati()
  {
    static const int chunk_size (1000);

    while (!_nodes_to_update.empty())
    {
      QString message ("status");
      while (message.size() < chunk_size && !_nodes_to_update.empty())
      {
        const QString hostname (_nodes_to_update.takeFirst());
        message.append (" ").append (hostname);
        _pending_updates << hostname;
      }

      _connection->push (message);
    }
  }

  boost::optional<int> node_state_widget::node_at (int x, int y) const
  {
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

  const state_description& node_state_widget::state (const boost::optional<QString>& name) const
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
  const node_type& node_state_widget::node (int index) const
  {
    return _nodes.at (index);
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

    for ( int i (row_begin * per_row + column_begin)
        ; i < node_count() && i < (row_end * per_row + column_end)
        ; ++i
        )
    {
      painter.drawPixmap ( rect_for_node (i, per_row).toRect()
                         , state (node (i).state())._pixmap
                         );
    }
  }

  bool node_state_widget::event (QEvent* event)
  {
    switch (event->type())
    {
    case QEvent::ToolTip:
      {
        QHelpEvent* help_event (static_cast<QHelpEvent*> (event));

        const boost::optional<int> node_index (node_at (help_event->x(), help_event->y()));
        if (node_index)
        {
          QToolTip::showText (help_event->globalPos(), QString ("%1: %2").arg (node (*node_index).hostname()). arg (node (*node_index).state().get_value_or ("unknown state")));
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

        const boost::optional<int> node_index (node_at (context_menu_event->x(), context_menu_event->y()));
        if (node_index)
        {
          QMenu context_menu;
          const node_type& n (node (*node_index));
          foreach (const QString& action, state (n.state())._actions)
          {
            fhg::util::qt::boost_connect<void (void)>
              ( context_menu.addAction
                (_long_action.contains (action) ? _long_action[action] : action)
              , SIGNAL (triggered())
              , _connection
              , boost::bind ( &async_tcp_communication::push
                            , _connection
                            , QString ("action %1 %2")
                            .arg (n.hostname()).arg (action)
                            )
              );
          }
          context_menu.exec (context_menu_event->globalPos());
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
}

#include <QApplication>

int main (int argc, char** argv)
{
  QApplication app (argc, argv);

  QScrollArea scroller;

  QWidget* inner (new QWidget (&scroller));
  QVBoxLayout* layout (new QVBoxLayout (inner));
  layout->addWidget (new prefix::node_state_widget);

  scroller.setWidget (inner);
  scroller.setWidgetResizable (true);
  scroller.show();

  return app.exec();
}
