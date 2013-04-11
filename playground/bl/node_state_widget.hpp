// bernd.loerwald@itwm.fraunhofer.de

//! \todo PREFIX
#ifndef PREFIX_NODE_STATE_WIDGET_HPP
#define PREFIX_NODE_STATE_WIDGET_HPP

#include <QColor>
#include <QList>
#include <QWidget>
#include <QMap>
#include <QList>

#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QPixmap>
#include <QTcpSocket>
#include <QTimer>

#include <stdexcept>

#include <boost/optional.hpp>
#include <fhg/util/parse/position.hpp>


namespace prefix
{
  static int counter (0);
  class node_type
  {
  public:
    node_type()
      : _state (boost::none)
      , _hostname (QString ("node%1.cluster").arg (++counter))
      , _details (boost::none)
    { }
    node_type (const QString& hostname)
      : _state (boost::none)
      , _hostname (hostname)
      , _details (boost::none)
    { }
    boost::optional<QString> _state;
    QString _hostname;
    boost::optional<QString> _details;
    const boost::optional<QString>& state() const { return _state; }
    void state (const boost::optional<QString>& state_) { _state = state_; }
    const boost::optional<QString>& details() const { return _details; }
    void details (const boost::optional<QString>& details_) { _details = details_; }
    const QString& hostname() const { return _hostname; }
    bool less_by_state (const node_type& other) const
    {
      return _state < other._state;
    }
    bool less_by_hostname (const node_type& other) const
    {
      return _hostname < other._hostname;
    }
    bool hostname_is (const QString& name) const
    {
      return hostname() == name;
    }
  };

  struct state_description
  {
    QStringList _actions;
    boost::optional<char> _character;
    QColor _brush;
    QColor _pen;

    QPixmap _pixmap;

    state_description ( const QStringList& actions = QStringList()
                      , boost::optional<char> = boost::none
                      , QColor brush = Qt::lightGray
                      , QColor pen = Qt::black
                      );
    void reset();
  };

  class async_tcp_communication : public QObject
  {
    Q_OBJECT;

  public:
    async_tcp_communication (QObject* parent = NULL)
      : QObject (parent)
    {
      connect (&_socket, SIGNAL (readyRead()), SLOT (may_read()));
      _socket.connectToHost ("localhost", 44451);
      if (!_socket.waitForConnected())
      {
        throw std::runtime_error (qPrintable (_socket.errorString()));
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

  class node_state_widget : public QWidget
  {
    Q_OBJECT;

  public:
    node_state_widget (QWidget* parent = NULL);

    virtual int heightForWidth (int) const;

  protected:
    virtual void paintEvent (QPaintEvent*);
    virtual bool event (QEvent*);

  private slots:
    void check_for_incoming_messages();
    void refresh_stati();

  private:
    QMap<QString, QString> _long_action;

    QList<QString> _pending_updates;
    QList<QString> _nodes_to_update;

    void update (int node);

    void possible_status (fhg::util::parse::position&);
    void action_description (fhg::util::parse::position&, const QString&);
    void layout_hint (fhg::util::parse::position&, const QString&);
    void status_update (fhg::util::parse::position&, const QString&);

    QVector<node_type> _nodes;
    QMap<QString, state_description> _states;

    const state_description& state (const boost::optional<QString>&) const;
    const node_type& node (int) const;
    boost::optional<int> node_at (int x, int y) const;
    int node_count() const;

    async_tcp_communication* _connection;
  };
}

#endif
