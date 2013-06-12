// bernd.loerwald@itwm.fraunhofer.de

//! \todo PREFIX
#ifndef PREFIX_NODE_STATE_WIDGET_HPP
#define PREFIX_NODE_STATE_WIDGET_HPP

#include <QColor>
#include <QList>
#include <QWidget>
#include <QMap>
#include <QList>

#include <QDateTime>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QPixmap>
#include <QTcpSocket>
#include <QListWidget>
#include <QTimer>

#include <stdexcept>

#include <boost/optional.hpp>
#include <fhg/util/parse/position.hpp>
#include <fhg/util/alphanum.hpp>

namespace prefix
{
  class node_type
  {
  public:
    node_type (const QString& hostname)
      : _state (boost::none)
      , _hostname (hostname)
      , _details (boost::none)
      , _watched (false)
    { }

    const bool& watched() const { return _watched; }
    void watched (const bool& watched_) { _watched = watched_; }

    const boost::optional<QString>& state() const { return _state; }
    void state (const boost::optional<QString>& state_)
    {
      _state = state_;
      _state_update_time = QDateTime::currentDateTime();
    }

    const QDateTime& state_update_time() const { return _state_update_time; }

    const boost::optional<QString>& details() const { return _details; }
    void details (const boost::optional<QString>& details_) { _details = details_; }

    const QString& hostname() const { return _hostname; }

  private:
    boost::optional<QString> _state;
    QDateTime _state_update_time;
    QString _hostname;
    boost::optional<QString> _details;
    bool _watched;
  };

  struct state_description
  {
    QStringList _actions;
    boost::optional<char> _character;
    QColor _brush;
    QColor _pen;
    bool _hidden;

    QPixmap _pixmap;

    state_description ( const QStringList& actions = QStringList()
                      , boost::optional<char> = boost::none
                      , QColor brush = Qt::lightGray
                      , QColor pen = Qt::black
                      );
    void reset();
  };

  class legend_entry : public QWidget
  {
    Q_OBJECT;

  public:
    legend_entry
      (const QString&, const state_description&, QWidget* parent = NULL);
  };

  class legend : public QWidget
  {
    Q_OBJECT;

  public:
    legend (QWidget* parent = NULL);

    const state_description& state (const boost::optional<QString>&) const;

  public slots:
    void update (const QString&);
    void states_add (const QString&, const QStringList&);
    void states_layout_hint_border (const QString&, const QColor&);
    void states_layout_hint_character (const QString&, const char&);
    void states_layout_hint_color (const QString&, const QColor&);
    void states_layout_hint_hidden (const QString&, const bool&);

  signals:
    void state_pixmap_changed (const QString&);

  private:
    QMap<QString, state_description> _states;
    QMap<QString, legend_entry*> _state_legend;
  };

  struct connection_error : public std::runtime_error
  {
    connection_error (const QTcpSocket& socket)
      : std::runtime_error (qPrintable (socket.errorString()))
    { }
  };

  class async_tcp_communication : public QObject
  {
    Q_OBJECT;

  public:
    async_tcp_communication
        (const QString& host, int port, QObject* parent = NULL)
      : QObject (parent)
    {
      connect (&_socket, SIGNAL (readyRead()), SLOT (may_read()));
      _socket.connectToHost (host, port);
      if (!_socket.waitForConnected())
      {
        throw connection_error (_socket);
      }

      QTimer* timer (new QTimer (this));
      connect (timer, SIGNAL (timeout()), SLOT (send_outstanding()));
      timer->start (100);
    }

    typedef QList<QString> messages_type;

    messages_type get()
    {
      messages_type messages;
      {
        const QMutexLocker lock (&_outstanding_incoming_lock);
        std::swap (messages, _outstanding_incoming);
      }
      return messages;
    }

    void push (const QString& message)
    {
      const QMutexLocker lock (&_outstanding_outgoing_lock);
      _outstanding_outgoing << message;
    }

  private slots:
    void send_outstanding()
    {
      messages_type messages;

      {
        const QMutexLocker lock (&_outstanding_outgoing_lock);
        std::swap (messages, _outstanding_outgoing);
      }

      foreach (const QString& message, messages)
      {
        _socket.write (qPrintable (message));
        _socket.write ("\n");
      }
    }

    void may_read()
    {
      if (!_socket.canReadLine())
      {
        return;
      }

      messages_type messages;

      while (_socket.canReadLine())
      {
        messages << _socket.readLine().trimmed();
      }

      const QMutexLocker lock (&_outstanding_incoming_lock);
      _outstanding_incoming << messages;
    }

  private:
    messages_type _outstanding_incoming;
    mutable QMutex _outstanding_incoming_lock;

    messages_type _outstanding_outgoing;
    mutable QMutex _outstanding_outgoing_lock;

    QTcpSocket _socket;
  };

  //! action_result
  enum result_code
  {
    okay,
    fail,
    warn,
  };

  struct action_argument_data
  {
    enum type
    {
      boolean,
      directory,
      duration,
      filename,
      integer,
      string,
    };

    action_argument_data (const QString& name);

    QString _name;
    boost::optional<type> _type;
    boost::optional<QString> _label;
    boost::optional<QString> _default;

    void append (fhg::util::parse::position&);
  };

  class communication : public QObject
  {
    Q_OBJECT;

  public:
    communication (const QString& host, int port, QObject* parent = NULL);

    void request_action ( const QString&
                        , const QString&
                        , const QMap<QString, boost::function<QString()> >&
                        );
    void request_layout_hint (const QString&);
    void request_action_description (const QStringList&);
    void request_hostlist();
    void request_status (QStringList);

    void pause();
    void resume();

  signals:
    void action_result ( const QString&
                       , const QString&
                       , const result_code&
                       , const boost::optional<QString>&
                       );
    void nodes (QStringList);
    void nodes_state_clear (const QString&);
    void nodes_details (const QString&, const QString&);
    void nodes_state (const QString&, const QString&);
    void states_actions_long_text (const QString&, const QString&);
    void states_actions_arguments
      (const QString&, const QList<action_argument_data>&);
    void states_add (const QString&, const QStringList&);
    void states_layout_hint_border (const QString&, const QColor&);
    void states_layout_hint_character (const QString&, const char&);
    void states_layout_hint_color (const QString&, const QColor&);
    void states_layout_hint_hidden (const QString&, const bool&);

  private slots:
    void check_for_incoming_messages();

  private:
    void possible_status (fhg::util::parse::position&);
    void action_description (fhg::util::parse::position&, const QString&);
    void layout_hint (fhg::util::parse::position&, const QString&);
    void status_update (fhg::util::parse::position&);
    void status_update_data (fhg::util::parse::position&, const QString&);
    void action_result (fhg::util::parse::position&);

    async_tcp_communication* _connection;

    QTimer* _timer;
  };

  class log_widget : public QListWidget
  {
    Q_OBJECT;

  public:
    log_widget (QWidget* parent = NULL);

    void critical (const QString&);
    void information (const QString&);
    void warning (const QString&);

  public slots:
    void follow (bool);
  };

  class node_state_widget : public QWidget
  {
    Q_OBJECT;

  public:
    node_state_widget ( const QString& host
                      , int port
                      , legend*
                      , log_widget*
                      , QWidget* parent = NULL
                      );

    virtual int heightForWidth (int) const;

  protected:
    virtual void paintEvent (QPaintEvent*);
    virtual bool event (QEvent*);

    virtual void mouseReleaseEvent (QMouseEvent*);

  private slots:
    void refresh_stati();
    void nodes (QStringList);
    void nodes_details (const QString&, const QString&);
    void nodes_state (const QString&, const QString&);
    void nodes_state_clear (const QString&);
    void states_actions_long_text (const QString&, const QString&);
    void states_actions_arguments
      (const QString&, const QList<action_argument_data>&);

    void update_nodes_with_state (const QString&);
    void trigger_action (const QStringList& hosts, const QString& action);

    void action_result ( const QString&
                       , const QString&
                       , const result_code&
                       , const boost::optional<QString>&
                       );

    void sort_by_name();
    void sort_by_state();

    void select_all();
    void clear_selection();

  signals:

  private:
    QMap<QString, QString> _long_action;
    QMap<QString, QList<action_argument_data> > _action_arguments;

    QList<QString> _pending_updates;
    QList<QString> _nodes_to_update;

    void update (int node);
    void update();

    QList<node_type> _nodes;
    QList<int> _selection;

    void add_to_selection (const int&);
    void remove_from_selection (const int&);

    boost::optional<int> _last_manual_selection;

    legend* _legend_widget;
    log_widget* _log;

    const state_description& state (const boost::optional<QString>&) const;
    const node_type& node (int) const;
    node_type& node (int);
    boost::optional<int> node_at (const QPoint&) const;
    int node_count() const;

    communication* _communication;

    QString _host;
  };
}

#endif
