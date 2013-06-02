// {petry,bernd.loerwald}@itwm.fraunhofer.de

#include "server.hpp"

#include "parse.hpp"

#include <fhg/util/parse/error.hpp>
#include <fhg/util/alphanum.hpp>

#include <iostream>

#include <QDebug>
#include <stdexcept>
#include <QStringList>
#include <QTimer>
#include <QMutexLocker>
#include <QFileSystemWatcher>
#include <QFile>
#include <QProcess>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/discrete_distribution.hpp>

namespace gspc
{
  namespace mon
  {
    server::server ( int port
                   , const QString& hostlist
                   , const QDir& hookdir
                   , QObject* parent)
      : QTcpServer (parent)
      , _hostlist (hostlist)
      , _hookdir (hookdir)
    {
      if (!listen (QHostAddress::Any, port))
      {
        throw std::runtime_error (qPrintable (errorString()));
      }

      _hookdir.makeAbsolute ();

      qDebug() << "listening on port" << serverPort()
               << "using hostlist" << _hostlist
               << "and hooks from" << _hookdir.absolutePath ()
        ;
    }

    server::~server ()
    {}

    void server::incomingConnection (int socket_descriptor)
    {
      gspc::mon::thread* t (new gspc::mon::thread ( socket_descriptor
                                                  , _hostlist
                                                  , _hookdir
                                                  )
                           );
      t->moveToThread (t);
      t->start();
    }

    thread::thread ( int socket_descriptor
                   , const QString& hostlist
                   , const QDir& hookdir
                   , QObject* parent
                   )
      : QThread (parent)
      , _socket_descriptor (socket_descriptor)
      , _socket (NULL)
      , _hookdir (hookdir)
    {
      QFileSystemWatcher* watcher (new QFileSystemWatcher (this));
      connect ( watcher, SIGNAL (fileChanged (const QString&))
              , this, SLOT (read_hostlist (const QString&))
              );

      watcher->addPath (hostlist);
      read_hostlist (hostlist);

      // initialize actions

      //!\todo scan _hookdir for executable  files, call "file --description" to
      // get the list of descriptions, maybe split path by "state-action"?

      _hooks ["reboot"].first = _hookdir.absoluteFilePath ("reboot");
      _hooks ["reboot"].second = "reboot the node";

      _hooks ["add"].first = _hookdir.absoluteFilePath ("add");
      _hooks ["add"].second = "add the node to the working set";

      _hooks ["remove"].first = _hookdir.absoluteFilePath ("remove");
      _hooks ["remove"].second = "remove the node to the working set";

      _actions ["down"] << "reboot";
      _actions ["available"] << "add";
      _actions ["unavailable"] << "reboot";
      _actions ["inuse"] << "remove";
    }

    thread::~thread () {}

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
      const char* states[] = {"down", "available", "unavailable", "inuse"};
      const double initial_state_probabilities[] = {
        0.1, 0.6, 0.2, 0.1
      };

      QString random_initial_state ()
      {
        static boost::mt19937 gen;
        static boost::random::discrete_distribution<> dist
          (initial_state_probabilities);

        return states [dist (gen)];
      }
    }

    void thread::read_hostlist (const QString& hostlist)
    {
      QFile file (hostlist);

      if (file.open (QIODevice::ReadOnly | QIODevice::Text))
      {
        const QMutexLocker lock (&_hosts_mutex);
        _hosts.clear();

        while (!file.atEnd())
        {
          _hosts.insert ( QString (file.readLine()).trimmed()
                        , qMakePair (random_initial_state(), 0)
                        );
        }
      }
    }

    namespace
    {
      void insert_into_map
      (fhg::util::parse::position& pos, QMap<QString, QString>* map)
      {
        const QString key (prefix::require::qstring (pos));
        prefix::require::token (pos, ":");
        map->insert (key, prefix::require::qstring (pos));
      }

      struct action_invocation
      {
        boost::optional<QString> _host;
        boost::optional<QString> _action;
        QMap<QString, QString> _arguments;

        void append (fhg::util::parse::position& pos)
        {
          pos.skip_spaces();

          if (pos.end() || (*pos != 'a' && *pos != 'h'))
          {
            throw fhg::util::parse::error::expected ("a' or 'host", pos);
          }

          switch (*pos)
          {
          case 'a':
            ++pos;
            {
              if (pos.end() || (*pos != 'c' && *pos != 'r'))
              {
                throw fhg::util::parse::error::expected ("ction' or 'rguments", pos);
              }

              switch (*pos)
              {
              case 'c':
                ++pos;
                pos.require ("tion");
                prefix::require::token (pos, ":");

                _action = prefix::require::qstring (pos);

                break;

              case 'r':
                ++pos;
                pos.require ("guments");
                prefix::require::token (pos, ":");

                prefix::require::list
                  (pos, boost::bind (insert_into_map, _1, &_arguments));

                break;
              }
            }

            break;

          case 'h':
            ++pos;
            pos.require ("ost");
            prefix::require::token (pos, ":");

            _host = prefix::require::qstring (pos);

            break;
          }
        }
      };
    }

    void thread::execute_action (fhg::util::parse::position& pos)
    {
      action_invocation invoc;
      prefix::require::list
        (pos, boost::bind (&action_invocation::append, &invoc, _1));

      if (!invoc._host || !invoc._action)
      {
        throw std::runtime_error ("action missing action name or host");
      }

      qDebug() << "execute" << *invoc._action << "for" << *invoc._host << "with arguments:";
      QMap<QString, QString>::const_iterator i (invoc._arguments.constBegin());
      while (i != invoc._arguments.constEnd())
      {
        qDebug() << ">" << i.key() << "="<< i.value();
        ++i;
      }

      const QMutexLocker lock (&_hosts_mutex);

      if (!_hosts.contains (*invoc._host))
      {
        _socket->write
          ( qPrintable ( QString
                       ("action_result: [(\"%1\", \"%2\"): [result: fail, message: \"Unknown host.\"],]\n")
                       .arg (*invoc._host)
                       .arg (*invoc._action)
                       )
          );

        return;
      }

      QString& state (_hosts[*invoc._host].first);
      int& last_change (_hosts[*invoc._host].second);

      if (*invoc._action == "reboot" && (state == "unavailable" || state == "available"))
      {
        state = "down";
        last_change = 0;

        _socket->write
          ( qPrintable ( QString
                       ("action_result: [(\"%1\", \"%2\"): [result: okay, message: \"Will reboot.\"],]\n")
                       .arg (*invoc._host)
                       .arg (*invoc._action)
                       )
          );
      }
      else if (*invoc._action == "add" && state == "available")
      {
        state = "inuse";
        last_change = 0;

        _socket->write
          ( qPrintable ( QString
                       ("action_result: [(\"%1\", \"%2\"): [result: okay, message: \"Added to working set.\"]]\n")
                       .arg (*invoc._host)
                       .arg (*invoc._action)
                       )
          );
      }
      else if (*invoc._action == "remove" && state == "inuse")
      {
        state = "available";
        last_change = 0;

        _socket->write
          ( qPrintable ( QString
                       ("action_result: [(\"%1\", \"%2\"): [result: okay, message: \"Removed from working set.\"]]\n")
                       .arg (*invoc._host)
                       .arg (*invoc._action)
                       )
          );
      }
      else
      {
        _socket->write
          ( qPrintable ( QString
                       ("action_result: [(\"%1\", \"%2\"): [result: fail, message: \"Can't execute %2 on host %1 in state %3.\"],]\n")
                       .arg (*invoc._host)
                       .arg (*invoc._action)
                       .arg (state)
                       )
          );
      }
    }

    QString thread::description (const QString& action)
    {
      if (_hooks.contains (action))
        return _hooks.value (action).second;
      throw std::runtime_error ("unknown action: " + action.toStdString ());
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
      const QString state (prefix::require::qstring (pos));
      _socket->write
        ( qPrintable ( QString
                     ("layout_hint: [\"%1\": [color: %2, border: 0,],]\n")
                     .arg (state)
                     .arg ( state == "down" ? 0xEF0A06
                          : state == "available" ? 0x23AEB8
                          : state == "unavailable" ? 0xFF5405
                          : state == "inuse" ? 0x155F22
                          : 0xFFFFFF
                          )
                     )
        );
    }

    namespace
    {
      struct less
        : public std::binary_function<QString, QString, bool>
      {
        bool operator() (const QString& left, const QString& right) const
        {
          return fhg::util::alphanum::less()
            (left.toStdString(), right.toStdString());
        }
      };
    }

    void thread::may_read()
    {
      while (_socket->canReadLine())
      {
        const std::string message
          (QString (_socket->readLine()).trimmed().toStdString());

        qDebug () << "got message: " << QString (message.c_str ());

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
            pos.require ("osts");

            {
              _socket->write ("hosts: [");

              const QMutexLocker lock (&_hosts_mutex);

              QList<QString> hosts (_hosts.keys());
              qStableSort (hosts.begin(), hosts.end(), less());

              foreach (const QString& host, hosts)
              {
                _socket->write (qPrintable (QString (" \"%1\",").arg (host)));
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
            pos.require ("ossible_status");

            {
              _socket->write ("possible_status: [");
              for (size_t i (0); i < status_count; ++i)
              {
                QString s (states [i]);

                _socket->write ("\"");
                _socket->write (qPrintable (s));
                _socket->write ("\":[");

                foreach (const QString& a, _actions [s])
                {
                  _socket->write ("\"");
                  _socket->write (qPrintable (a));
                  _socket->write ("\", ");
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

    namespace
    {
      bool chance (const float percentage)
      {
        return float (qrand()) / float (RAND_MAX) < percentage / 100.0;
      }
    }

    void thread::send_some_status_updates()
    {
      const QMutexLocker lock (&_pending_status_updates_mutex);
      while (!_pending_status_updates.empty() && chance (99.9))
      {
        const QString host (_pending_status_updates.takeFirst());

        const QMutexLocker lock (&_hosts_mutex);

        if (!_hosts.contains (host))
        {
          qDebug() << "requested state update for unknown host";
          continue;
        }

        QString& state (_hosts[host].first);
        int& last_change (_hosts[host].second);

        if (state == "down" && last_change > 10 && chance (10.0 * (last_change - 10)))
        {
          state = "available";
          last_change = 0;
        }
        else if (chance (0.1) && (state == "available" || state == "inuse"))
        {
          state = "unavailable";
          last_change = 0;
        }
        else if (chance (0.05) && state != "down")
        {
          state = "down";
          last_change = 0;
        }
        else
        {
          ++last_change;
        }

        if (state == "inuse")
        {
          static const char* transition[] = {"map", "reduce", "collect"};
          _socket->write ( qPrintable ( QString ("status: [\"%1\": [state:\"%2\", details:\"%3\"]]\n")
                                      .arg (host)
                                      .arg (state)
                                      .arg (transition[qrand()%3])
                                      )
                         );
        }
        else
        {
          _socket->write ( qPrintable ( QString ("status: [\"%1\": [state:\"%2\"]]\n")
                                      .arg (host)
                                      .arg (state)
                                      )
                         );
        }
      }
    }
  }
}
