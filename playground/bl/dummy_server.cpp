// bernd.loerwald@itwm.fraunhofer.de

#include "dummy_server.hpp"

#include <QApplication>
#include <QDebug>
#include <stdexcept>
#include <QStringList>
#include <QTimer>
#include <QMutexLocker>

server::server (QObject* parent)
  : QTcpServer (parent)
{
  if (!listen (QHostAddress::LocalHost, 44451))
  {
    throw std::runtime_error (qPrintable (errorString()));
  }

  qDebug() << "listening on port " << serverPort();
}

void server::incomingConnection (int socket_descriptor)
{
  ::thread* t (new ::thread (socket_descriptor));
  t->moveToThread (t);
  t->start();
}

thread::thread (int socket_descriptor, QObject* parent)
  : QThread (parent)
  , _socket_descriptor (socket_descriptor)
  , _socket (NULL)
{ }

void thread::run()
{
  _socket = new QTcpSocket;
  _socket->setSocketDescriptor (_socket_descriptor);

  connect (_socket, SIGNAL (readyRead()), this, SLOT (may_read()));

  QTimer* timer (new QTimer);
  connect (timer, SIGNAL (timeout()), this, SLOT (send_some_status_updates()));
  timer->start (500);

  exec();

  delete _socket;
  delete timer;
}

namespace
{
  const size_t status_count (4);
  const char* states[] = {"down", "available", "unavailable", "used"};
  const char* actions[] = {NULL, "add_to_working_set", "reboot", "remove_from_working_set foo"};
}

void thread::may_read()
{
  while (_socket->canReadLine())
  {
    const QString line (_socket->readLine().trimmed());
    // qDebug() << "server: read:" << line;

    const QStringList tokens (line.split (' '));

    if (tokens[0] == "host_list")
    {
      _socket->write ("hosts: [");
      int count (qrand() % 1000 + 5000);
      while (count--)
      {
        _socket->write (qPrintable (QString (" \"node%1.cluster\",").arg (count)));
      }
      _socket->write ("]\n");
    }
    else if (tokens[0] == "possible_status_list")
    {
      _socket->write ("possible_status: [");
      for (size_t i (0); i < status_count; ++i)
      {
        _socket->write ("\"");
        _socket->write (states[i]);
        _socket->write ("\":[");
        if (actions[i])
        {
          _socket->write (" \"");
          _socket->write (actions[i]);
          _socket->write ("\",");
        }
        _socket->write ("],");
      }
      _socket->write ("]\n");
    }
    else if (tokens[0] == "status")
    {
      const QMutexLocker lock (&_pending_status_updates_mutex);
      foreach (const QString& host, tokens.mid (1))
      {
        _pending_status_updates.append (host);
      }
    }
    else if (tokens[0] == "action")
    {
      qDebug() << "execute action " << tokens[2] << " on " << tokens[1];
    }
    else if (tokens[0] == "describe_action")
    {
      foreach (const QString& action, tokens.mid (1))
      {
        _socket->write
          ( qPrintable ( QString
                         ("action_description: [\"%1\": [long_text: \"%2\",],]\n")
                       .arg (action)
                       .arg ( action == "add_to_working_set"
                            ? "add this node to the working set"
                            : action == "reboot"
                            ? "reboot node"
                            : action == "remove_from_working_set"
                            ? "remove this node from the working set"
                            : action == "foo"
                            ? "foo bar"
                            : throw std::runtime_error ("unknown action name")
                            )
                       )
          );
      }
    }
    else if (tokens[0] == "layout_hint")
    {
      _socket->write
        ( qPrintable ( QString
                       ("layout_hint: [\"%1\": [color:0x%2,border:0x%3,],]\n")
                     .arg (tokens[1])
                     .arg (qrand() ^ qrand(), 0, 16)
                     .arg (qrand() ^ qrand(), 0, 16)
                     )
        );
    }
  }
}

void thread::send_some_status_updates()
{
  const QMutexLocker lock (&_pending_status_updates_mutex);
  while (!_pending_status_updates.empty() && qrand() % 1000)
  {
    double p (0.8);
    int q (-1);
    for (int k (status_count - 1); k >= 0 && q < 0; --k, p += p / 10)
    {
      if (double(qrand()) < p * double(RAND_MAX))
      {
        q = k;
      }
    }

    if (q == 3)
    {
      static const char* transition[] = {"map", "reduce", "collect"};
      _socket->write ( qPrintable ( QString ("status: [\"%1\": [state:\"%2\", details:\"%3\"]]\n")
                                  .arg (_pending_status_updates.takeFirst())
                                  .arg (states[q])
                                  .arg (transition[qrand()%3])
                                  )
                     );
    }
    else
    {
      _socket->write ( qPrintable ( QString ("status: [\"%1\": [state:\"%2\"]]\n")
                                  .arg (_pending_status_updates.takeFirst())
                                  .arg (states[q])
                                  )
                     );
    }
  }
}

int main (int argv, char** argc)
{
  QApplication app (argv, argc);

  new server;

  app.exec();
}
