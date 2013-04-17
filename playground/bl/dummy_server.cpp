// bernd.loerwald@itwm.fraunhofer.de

#include "dummy_server.hpp"

#include "parse.hpp"

#include <fhg/util/parse/error.hpp>

#include <iostream>

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
  const char* actions[] = {NULL, "add_to_working_set", "reboot", "remove_from_working_set\", \"foo"};

  const char* description (const QString& action)
  {
    return action == "add_to_working_set"
      ? "add this node to the working set"
      : action == "reboot"
      ? "reboot {hostname}"
      : action == "remove_from_working_set"
      ? "remove this node from the working set"
      : action == "foo"
      ? "foo bar"
      : throw std::runtime_error ("unknown action name");
  }
}

void thread::execute_action (fhg::util::parse::position& pos)
{
  const QString host (prefix::require::qstring (pos));
  prefix::require::token (pos, ":");
  const QString action (prefix::require::qstring (pos));
  qDebug() << "execute" << action << "for" << host;
}

void thread::send_action_description (fhg::util::parse::position& pos)
{
  const QString action (prefix::require::qstring (pos));
  _socket->write
    ( qPrintable ( QString
                   ("action_description: [\"%1\": [long_text: \"%2\",],]\n")
                 .arg (action)
                 .arg (description (action))
                 )
    );
}

void thread::send_layout_hint (fhg::util::parse::position& pos)
{
  _socket->write
    ( qPrintable ( QString
                 ("layout_hint: [\"%1\": [color:0x%2,border:0x%3,],]\n")
                 .arg (prefix::require::qstring (pos))
                 .arg (qrand() ^ qrand(), 0, 16)
                 .arg (qrand() ^ qrand(), 0, 16)
                 )
    );
}

void thread::may_read()
{
  while (_socket->canReadLine())
  {
    const std::string message
      (QString (_socket->readLine()).trimmed().toStdString());
    fhg::util::parse::position pos (message);

    try
    {
      pos.skip_spaces();

      if ( pos.end()
        || ( *pos != 'a' && *pos != 'd' && *pos != 'h'
          && *pos != 'l' && *pos != 'p' && *pos != 's'
           )
         )
      {
        throw fhg::util::parse::error::expected ("packet", pos);
      }

      switch (*pos)
      {
      case 'a':
        ++pos;
        pos.require ("ction");
        prefix::require::token (pos, ":");

        prefix::require::list (pos, boost::bind (&thread::execute_action, this, _1));

        break;

      case 'd':
        ++pos;
        pos.require ("escribe_action");
        prefix::require::token (pos, ":");

        prefix::require::list
          (pos, boost::bind (&thread::send_action_description, this, _1));

        break;

      case 'h':
        ++pos;
        pos.require ("ost_list");

        {
          _socket->write ("hosts: [");
          int count (qrand() % 1000 + 5000);
          while (count--)
          {
            _socket->write (qPrintable (QString (" \"node%1.cluster\",").arg (count)));
          }
          _socket->write ("]\n");
        }

        break;

      case 'l':
        ++pos;
        pos.require ("ayout_hint");
        prefix::require::token (pos, ":");

        prefix::require::list (pos, boost::bind (&thread::send_layout_hint, this, _1));

        break;

      case 'p':
        ++pos;
        pos.require ("ossible_status_list");

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

        break;

      case 's':
        ++pos;
        pos.require ("tatus");
        prefix::require::token (pos, ":");

        {
          const QMutexLocker lock (&_pending_status_updates_mutex);
          prefix::require::list
            ( pos
            , boost::bind ( &QStringList::push_back
                          , &_pending_status_updates
                          , boost::bind (prefix::require::qstring, _1)
                          )
            );
        }
      }
    }
    catch (const std::runtime_error& ex)
    {
      //! \todo Report back to client?
      std::cerr << "PARSE ERROR: " << ex.what() << "\nmessage: " << message << "\nrest: " << pos.rest() << "\n";
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
