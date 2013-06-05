// {petry,bernd.loerwald}@itwm.fraunhofer.de

#include "server.hpp"

#include "parse.hpp"

#include <stdlib.h>

#include <fhg/util/parse/error.hpp>
#include <fhg/util/alphanum.hpp>

#include <iostream>
#include <sstream>

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
    server::server ( const QString &workdir
                   , int port
                   , const QString& hostlist
                   , const QDir& hookdir
                   , QObject* parent)
      : QTcpServer (parent)
      , _workdir (workdir)
      , _hostlist (hostlist)
      , _hookdir (hookdir)
    {
      if (!listen (QHostAddress::Any, port))
      {
        throw std::runtime_error (qPrintable (errorString()));
      }

      _workdir.makeAbsolute ();
      _hookdir.makeAbsolute ();

      {
        std::string s (_workdir.absolutePath ().toStdString ());
        ::setenv ("GSPCMON_WORKDIR", s.c_str (), 1);
      }

      qDebug() << "listening on port" << serverPort()
               << "running with workdir" << _workdir.absolutePath ()
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
      _hooks ["remove"].second = "remove the node from the working set";

      _hooks ["shutdown"].first = _hookdir.absoluteFilePath ("shutdown");
      _hooks ["shutdown"].second = "take the node offline";

      _hooks ["status"].first = _hookdir.absoluteFilePath ("status");
      _hooks ["status"].second = "query the status of a node";

      _hooks ["start"].first = _hookdir.absoluteFilePath ("start");
      _hooks ["start"].second = "start the RTM";

      add_state ("down", "0xEF0A06").actions            << "reboot";

      add_state ("free", "0x23AEB8").actions          << "start";
      add_state ("free/reserved", "0x23AE88").actions << "start";

      add_state ("unavailable", "0xFF5405");

      add_state ("addable", "0x23AEB8").actions            << "add";
      add_state ("addable/reserved", "0x23AE88").actions   << "add";

      add_state ("master", "0x155F22").actions << "stop";
      add_state ("inuse",  "0x155F22").actions << "remove";
    }

    thread::state_info_t & thread::add_state ( QString const &name
                                             , QString const &color
                                             , int border
                                             )
    {
      state_info_t s;
      s.name = name;
      s.color = color;
      s.border = border;

      _states [name] = s;
      return _states [name];
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

    void thread::read_hostlist (const QString& hostlist)
    {
      QFile file (hostlist);

      if (file.open (QIODevice::ReadOnly | QIODevice::Text))
      {
        const QMutexLocker lock (&_hosts_mutex);
        _hosts.clear();

        while (!file.atEnd())
        {
          host_state_t hs;
          hs.state = "down";
          hs.age = -1;
          hs.details = "";

          _hosts.insert ( QString (file.readLine()).trimmed()
                        , hs
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

    /*
      hooks/add <node001> <node002> <node003>
           node001 0 inuse node001 added
           node002 1 inuse node001 already in use
           node003 2 down  no such node
    */

    struct hook_partial_result_t
    {
      QString host;
      int exit_code; // 0 = success, 1 = warning, >= 2 = failed
      QString state;
      QString message;
    };

    int call_action_hook ( QString program
                         , QStringList nodes
                         , QMap<QString, hook_partial_result_t> &result
                         , QString &error_reason
                         , QObject *parent = 0
                         )
    {
      int rc = 0;

      QStringList args;
      foreach (const QString& host, nodes)
      {
        args << host;
      }

      QProcess *p = new QProcess (parent);
      p->start (program, args);
      if (p->waitForStarted ())
      {
        p->waitForFinished ();

        while (p->bytesAvailable ())
        {
          QString line = QString (p->readLine ()).trimmed ();
          std::stringstream sstr (line.toStdString ());
          hook_partial_result_t res;

          {
            std::string s;
            sstr >> s; res.host = s.c_str ();
          }
          sstr >> res.exit_code;
          {
            std::string s;
            sstr >> s; res.state = s.c_str ();
          }
          {
            std::string s;
            std::getline (sstr, s);
            res.message = s.c_str ();
          }

          result [res.host] = res;
        }

        p->setReadChannel (QProcess::StandardError);
        while (p->bytesAvailable ())
        {
          // take last line
          error_reason = QString (p->readLine ()).trimmed ();
        }

        rc = p->exitCode ();

        if (0 != rc)
        {
          qDebug () << program << "failed:" << rc;
          if (error_reason.isEmpty ())
          {
            error_reason = "unknown reason";
          }
        }
      }
      else
      {
        switch (p->error ())
        {
        case QProcess::FailedToStart:
          error_reason = "Permission denied / No such file or directory";
          rc = 127;
          break;
        case QProcess::Crashed:
          error_reason = "crashed";
          rc = 137;
          break;
        case QProcess::Timedout:
          error_reason = "timedout";
          rc = 128;
          break;
        case QProcess::WriteError:
          error_reason = "could not write to stream";
          rc = 141;
          break;
        case QProcess::ReadError:
          error_reason = "could not read from stream";
          rc = 141;
          break;
        case QProcess::UnknownError:
          error_reason = "unknown reason";
          rc = 128;
          break;
        }
      }

      delete p;

      return rc;
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

      host_state_t & hs = _hosts [*invoc._host];

      QString& state = hs.state;
      int& age = hs.age;
      QString & details = hs.details;
      details = "";

      QMap<QString, hook_partial_result_t> results;
      QString error_reason;
      int ec = call_action_hook ( _hooks [*invoc._action].first
                                , QStringList () << *invoc._host
                                , results
                                , error_reason
                                , this
                                );
      if (0 == ec)
      {
        hook_partial_result_t & pres = results [*invoc._host];

        state = pres.state;
        age = 0;

        if (0 == pres.exit_code)
        {
          _socket->write
            ( qPrintable ( QString
                         ("action_result: [(\"%1\", \"%2\"): [result: okay, message: \"%3\"]]\n")
                         .arg (pres.host)
                         .arg (*invoc._action)
                         .arg (pres.message)
                         )
            );
        }
        else if (1 == pres.exit_code)
        {
          _socket->write
            ( qPrintable ( QString
                         ("action_result: [(\"%1\", \"%2\"): [result: warn, message: \"%3\"]]\n")
                         .arg (pres.host)
                         .arg (*invoc._action)
                         .arg (pres.message)
                         )
            );
        }
        else
        {
          _socket->write
            ( qPrintable ( QString
                         ("action_result: [(\"%1\", \"%2\"): [result: fail, message: \"error %3: %4\"],]\n")
                         .arg (*invoc._host)
                         .arg (*invoc._action)
                         .arg (pres.exit_code)
                         .arg (pres.message)
                         )
            );
        }
      }
      else
      {
        _socket->write
          ( qPrintable ( QString
                       ("action_result: [(\"%1\", \"%2\"): [result: fail, message: \"'%2' on '%1' in state '%3' failed: %4: %5\"],]\n")
                       .arg (*invoc._host)
                       .arg (*invoc._action)
                       .arg (state)
                       .arg (ec)
                       .arg (error_reason)
                       )
          );

        qDebug () << "new state for host" << *invoc._host << "is" << state;
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
      const state_info_t &state = _states [prefix::require::qstring (pos)];

      _socket->write
        ( qPrintable ( QString
                     ("layout_hint: [\"%1\": [color: %2, border: %3,],]\n")
                     .arg (state.name)
                     .arg (state.color)
                     .arg (state.border)
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
              QList<QString> states (_states.keys());
              foreach (const QString& s, states)
              {
                _socket->write ("\"");
                _socket->write (qPrintable (s));
                _socket->write ("\":[");

                foreach (const QString& a, _states [s].actions)
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

    void thread::send_status_updates (QStringList const &hosts)
    {
      const QMutexLocker lock (&_hosts_mutex);
      foreach (const QString& host, hosts)
      {
        const host_state_t & hs = _hosts [host];
        send_status (host, hs.state, hs.details);
      }
    }

    void thread::send_status ( QString const &host
                             , QString const &state
                             , QString const &details
                             )
    {
      if (details.isEmpty ())
      {
        _socket->write ( qPrintable ( QString ("status: [\"%1\": [state:\"%2\"]]\n")
                                    .arg (host)
                                    .arg (state)
                                    )
                       );
      }
      else
      {
        _socket->write ( qPrintable ( QString ("status: [\"%1\": [state:\"%2\", details:\"%3\"]]\n")
                                    .arg (host)
                                    .arg (state)
                                    .arg (details)
                                    )
                       );
      }
    }

    void thread::send_some_status_updates()
    {
      QStringList to_query;
      QStringList to_send;

      {
        const QMutexLocker lock (&_pending_status_updates_mutex);
        while (!_pending_status_updates.empty ())
        {
          const QString host (_pending_status_updates.takeFirst ());

          const QMutexLocker lock (&_hosts_mutex);

          if (!_hosts.contains (host))
          {
            continue;
          }
          to_send << host;

          host_state_t & hs = _hosts[host];

          if (hs.age == -1 || hs.age > 10)
          {
            to_query << host;
          }
          else
          {
            ++hs.age;
          }
        }
      }

      if (not to_query.isEmpty ())
      {
        QMap<QString, hook_partial_result_t> results;
        QString error_reason;
        int ec = call_action_hook ( _hooks ["status"].first
                                  , to_query
                                  , results
                                  , error_reason
                                  , this
                                  );

        if (0 == ec)
        {
          const QMutexLocker lock (&_hosts_mutex);

          QList<QString> updated (results.keys());
          foreach (const QString& host, updated)
          {
            host_state_t & hs = _hosts [host];
            hook_partial_result_t & res = results [host];

            hs.state = res.state;
            hs.details = res.message.trimmed ();
            hs.age = 0;
          }
        }
        else
        {
          const QMutexLocker lock (&_hosts_mutex);
          foreach (const QString& host, to_query)
          {
            host_state_t & hs = _hosts[host];
            hs.state = "down";
            hs.details = "unknown";
            hs.age = 0;
          }
        }

      }

      send_status_updates (to_send);
    }
  }
}
