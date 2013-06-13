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
#include <QBuffer>
#include <QtCore>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/discrete_distribution.hpp>

namespace gspc
{
  namespace mon
  {
    server::server ( int port
                   , const QString& hostlist
                   , const QDir& hookdir
                   , const QString &workdir
                   , QObject* parent)
      : QTcpServer (parent)
      , _hostlist (hostlist)
      , _hookdir (hookdir)
      , _workdir(workdir)
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
                                                  , _workdir
                                                  )
                           );
      t->moveToThread (t);
      t->start();
    }

    thread::thread ( int socket_descriptor
                   , const QString& hostlist
                   , const QDir& hookdir
                   , const QString &workdir
                   , QObject* parent
                   )
      : QThread (parent)
      , _socket_descriptor (socket_descriptor)
      , _socket (NULL)
      , _hookdir (hookdir)
      , _workdir (workdir)
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

      add_action ("add", "add the node to the working set").params
        << parameter_info_t ("walltime", "Walltime in hours", "duration", QString ("4"))
        ;
      add_action ("restart", "restart missing workers");

      add_action ("remove", "remove the node from the working set");

      add_action ("status", "query the status of a node");

      add_action ("start", "start the RTM").params
        << parameter_info_t ("workdir", "Working directory", "directory")
        << parameter_info_t ("jobdesc", "Job description", "filename")
        << parameter_info_t ("nresult", "Maximum number of partial results", "string", QString ("factor 4.0"))
        << parameter_info_t ("updates", "Update interval", "integer", QString ("10"))
        << parameter_info_t ("atonce", "Shots at once", "integer", QString ("4"))
        << parameter_info_t ("walltime", "Walltime in hours", "duration", QString ("4"))
        ;

      add_action ("config", "change runtime parameters").params
        << parameter_info_t ("nresult", "Maximum number of partial results", "string")
        << parameter_info_t ("updates", "Update interval", "integer")
        << parameter_info_t ("atonce", "Shots at once", "integer")
        ;
      add_action ("killworker", "Simulate a worker failure")
        ;

      add_action ("query", "query current job-status");

      add_action ("stop", "stop the RTM");

      add_state ("down", 0x141414);

      add_state ("failed", 0xCC0000).actions        << "remove" << "restart";
      add_hidden_state ("master/failed", 0xFF0000).actions
        << "query"
        << "config"
        << "restart"
        << "stop"
        ;

      add_state ("free", 0x264673).actions          << "start";
      add_state ("free/reserved", 0xD2DF26).actions << "start";

      add_state ("queued", 0x004747).actions << "remove";
      add_state ("unavailable", 0x757575);

      add_hidden_state ("addable", 0x264673).actions            << "add";
      add_hidden_state ("addable/reserved", 0xD2DF26).actions   << "add";

      add_state ("master", 0x177D12).actions
        << "query"
        << "config"
        << "stop"
        << "killworker"
        ;
      add_state ("inuse",  0x1B590D).actions << "remove" << "killworker";
    }

    action_info_t & thread::add_action ( QString const &name
                                       , QString const &desc
                                       )
    {
      action_info_t ai;
      ai.name = name;
      ai.path = _hookdir.absoluteFilePath (name);
      ai.desc = desc;

      _actions [name] = ai;
      return _actions [name];
    }

    state_info_t & thread::add_state ( QString const &name
                                     , int color
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

    state_info_t & thread::add_hidden_state ( QString const &name
                                            , int color
                                            , int border
                                            )
    {
      state_info_t s;
      s.hidden = true;
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
      connect (timer, SIGNAL (timeout()), this, SLOT (send_some ()));
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

    int thread::call_action ( action_info_t const &ai
                            , QMap<QString, QString> const &params
                            , QSet<QString> const &nodes
                            , QMap<QString, action_result_t> &result
                            )
    {
      QString error_reason;
      int rc = 0;

      QStringList args;
      QMap<QString, QString>::const_iterator i (params.constBegin());
      while (i != params.constEnd())
      {
        args << QString ("%1=%2").arg (i.key ()).arg (i.value ());
        ++i;
      }

      // hack: set workdir if not given
      if (not params.contains ("workdir"))
      {
        args << QString ("workdir=%1").arg (_workdir);
      }

      foreach (const QString& node, nodes)
      {
        args << QString ("node=%1").arg (node);
      }

      qDebug() << "executing" << ai.path << args;

      QProcess *p_ptr = new QProcess;
      QProcess &p = *p_ptr;
      p.start (ai.path, args);
      if (p.waitForStarted (-1))
      {
        p.waitForFinished (-1);

        QByteArray bytes;

        bytes = p.readAllStandardOutput ();

        QBuffer buf (&bytes);
        buf.open(QIODevice::ReadOnly);

        while (buf.canReadLine())
        {
          QString line = QString (buf.readLine ()).trimmed ();
          std::stringstream sstr (line.toStdString ());
          action_result_t res;

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

        error_reason = QString (p.readAllStandardError().replace('\n', ' '));

        rc = p.exitCode ();

        if (0 != rc)
        {
          qDebug () << ai.path << "failed:" << rc << ":" << error_reason;
          if (error_reason.isEmpty ())
          {
            error_reason = "unknown reason";
          }
        }
        else
        {
          // hack: cache the workdir when we see a 'start' action
          if (ai.name == "start")
          {
            _workdir = params ["workdir"];
          }
          else if (ai.name == "stop")
          {
            _workdir = "";
          }
        }
      }
      else
      {
        QString error_reason;
        int rc;

        switch (p.error ())
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

      if (0 != rc)
      {
        foreach (const QString& node, nodes)
        {
          action_result_t res;
          res.host = node;
          res.exit_code = rc;
          res.state = "-";
          res.message = error_reason;
          result [node] = res;
        }
      }

      delete p_ptr;

      return rc;
    }

    static action_request_result_t s_run_request (thread *thrd, action_request_t const &req)
    {
      return thrd->run_request (req);
    }

    void thread::write_to_socket (QString const &s)
    {
      const QMutexLocker lock (&_socket_mutex);
      _socket->write (qPrintable (s));
      //      _socket->flush ();
    }

    action_request_result_t thread::run_request (action_request_t const &req)
    {
      action_request_result_t result;
      result.action = req.action;

      call_action ( _actions [req.action]
                  , req.params
                  , req.hosts
                  , result.result
                  );

      return result;
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

      {
        const QMutexLocker lock (&_hosts_mutex);

        if (!_hosts.contains (*invoc._host))
        {
          write_to_socket
            ( qPrintable ( QString
                         ("action_result: [(\"%1\", \"%2\"): [result: fail, message: \"Unknown host.\"],]\n")
                         .arg (*invoc._host)
                         .arg (*invoc._action)
                         )
            );
        return;
        }
      }

      action_request_t req;
      req.action = *invoc._action;
      req.hosts << *invoc._host;
      req.params = invoc._arguments;

      QFuture<action_request_result_t> *f
        = new QFuture<action_request_result_t>(QtConcurrent::run (s_run_request, this, req));

      {
        const QMutexLocker lock (&_action_results_mutex);
        _action_results << f;
      }
    }

    QString thread::description (const QString& action)
    {
      if (_actions.contains (action))
        return _actions.value (action).desc;
      throw std::runtime_error ("unknown action: " + action.toStdString ());
    }

    void thread::send_action_description (fhg::util::parse::position& pos)
    {
      const QString action (prefix::require::qstring (pos));

      action_info_t const &ai = _actions [action];

      const QMutexLocker lock (&_socket_mutex);

      _socket->write
        (qPrintable (QString ("action_description: [\"%1\": ").arg (ai.name)));

      _socket->write ("[");

      _socket->write
        (qPrintable (QString ("long_text: \"%1\", ").arg (ai.desc)));

      _socket->write
        (qPrintable (QString ("long_text: \"%1\", ").arg (ai.desc)));

      _socket->write ("arguments: [");

      foreach (const parameter_info_t& param, ai.params)
      {
        _socket->write
          (qPrintable (QString ("\"%1\": [").arg (param.name)));

        if (param.dflt)
        {
          _socket->write
            (qPrintable (QString ("default: \"%1\", ").arg (*param.dflt)));
        }

        _socket->write
          (qPrintable (QString ("type: %2, label: \"%3\", ")
                      .arg (param.type)
                      .arg (param.label)
                      )
          );

        _socket->write ("], ");
      }

      _socket->write ("]");
      _socket->write (",]");
      _socket->write (",]\n");
    }

    void thread::send_layout_hint (fhg::util::parse::position& pos)
    {
      const state_info_t &state = _states [prefix::require::qstring (pos)];

      write_to_socket
        ( qPrintable ( QString
                     ("layout_hint: [\"%1\": [color: %2, border: %3, hidden: %4,],]\n")
                     .arg (state.name)
                     .arg (state.color)
                     .arg (state.border)
                     .arg (state.hidden ? "true" : "false")
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

              QList<QString> hosts;
              {
                const QMutexLocker lock (&_hosts_mutex);
                hosts = _hosts.keys ();
              }
              qStableSort (hosts.begin(), hosts.end(), less());

              const QMutexLocker lock (&_socket_mutex);
              _socket->write ("hosts: [");
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
              write_to_socket ("possible_status: [");
              QList<QString> states (_states.keys());
              foreach (const QString& s, states)
              {
                write_to_socket ("\"");
                write_to_socket (qPrintable (s));
                write_to_socket ("\":[");

                foreach (const QString& a, _states [s].actions)
                {
                  write_to_socket ("\"");
                  write_to_socket (qPrintable (a));
                  write_to_socket ("\", ");
                }
                write_to_socket ("],");
              }
              write_to_socket ("]\n");
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
        write_to_socket ( qPrintable ( QString ("status: [\"%1\": [state:\"%2\"]]\n")
                                    .arg (host)
                                    .arg (state)
                                    )
                       );
      }
      else
      {
        write_to_socket ( qPrintable ( QString ("status: [\"%1\": [state:\"%2\", details:\"%3\"]]\n")
                                    .arg (host)
                                    .arg (state)
                                    .arg (details)
                                    )
                       );
      }
    }

    void thread::update_status (action_request_result_t const &res)
    {
      QList<QString> updated (res.result.keys());

      foreach (const QString& host, updated)
      {
        action_result_t const & r = res.result [host];
        {
          const QMutexLocker lock (&_hosts_mutex);
          host_state_t & hs = _hosts [host];

          if (r.state != "-")
            hs.state = r.state;
          hs.details = r.message.trimmed ();
          hs.age = 0;
        }
      }

      send_status_updates (updated);
    }

    void thread::handle_action_result (action_request_result_t const & res)
    {
      QList<QString> updated (res.result.keys());

      foreach (const QString& host, updated)
      {
        host_state_t hs;
        {
          const QMutexLocker lock (&_hosts_mutex);
          hs = _hosts [host];
        }

        action_result_t const & r = res.result [host];

        if (r.state != "-")
          hs.state = r.state;
        hs.age = 0;

        {
          const QMutexLocker lock (&_hosts_mutex);
          _hosts [host] = hs;
        }

        if (0 == r.exit_code)
        {
          write_to_socket
            (qPrintable ( QString
                        ("action_result: [(\"%1\", \"%2\"): [result: okay, message: \"%3\"]]\n")
                        .arg (host)
                        .arg (res.action)
                        .arg (r.message)
                        )
            );
        }
        else if (1 == r.exit_code)
        {
          write_to_socket
            (qPrintable ( QString
                        ("action_result: [(\"%1\", \"%2\"): [result: warn, message: \"%3\"]]\n")
                        .arg (host)
                        .arg (res.action)
                        .arg (r.message)
                        )
            );
        }
        else
        {
          write_to_socket
            ( qPrintable ( QString
                         ("action_result: [(\"%1\", \"%2\"): [result: fail, message: \"error %3: %4\"],]\n")
                         .arg (host)
                         .arg (res.action)
                         .arg (r.exit_code)
                         .arg (r.message)
                         )
            );
        }

        {
          const QMutexLocker lock (&_pending_status_updates_mutex);
          _pending_status_updates << host;
        }
      }
    }

    void thread::send_action_request_result (action_request_result_t const & res)
    {
      if (res.action == "status")
      {
        update_status (res);
      }
      else
      {
        handle_action_result (res);
      }
    }

    void thread::send_some ()
    {
      {
        const QMutexLocker lock (&_action_results_mutex);
        QSet<QFuture<action_request_result_t>* >::iterator i (_action_results.begin ());
        while (i != _action_results.end ())
        {
          QFuture<action_request_result_t> *f = *i;
          if (f->isFinished ())
          {
            send_action_request_result (f->result ());
            i = _action_results.erase (i);
            delete f;
          }
          else
          {
            ++i;
          }
        }
      }

      QSet<QString> to_query;
      {
        const QMutexLocker lock (&_pending_status_updates_mutex);
        while (!_pending_status_updates.empty ())
        {
          const QString host (_pending_status_updates.takeFirst ());
          {
            const QMutexLocker lock (&_hosts_mutex);
            if (!_hosts.contains (host))
              continue;
          }

          to_query << host;

          if (to_query.size () >= 8)
          {
            action_request_t req;
            req.action = "status";
            req.hosts = to_query;

            to_query.clear ();

            QFuture<action_request_result_t> *f
              = new QFuture<action_request_result_t>(QtConcurrent::run (s_run_request, this, req));

            {
              const QMutexLocker lock (&_action_results_mutex);
              _action_results << f;
            }
          }
        }
      }

      if (not to_query.isEmpty ())
      {
        action_request_t req;
        req.action = "status";
        req.hosts = to_query;

        to_query.clear ();

        QFuture<action_request_result_t> *f
          = new QFuture<action_request_result_t>(QtConcurrent::run (s_run_request, this, req));

        const QMutexLocker lock (&_action_results_mutex);
        _action_results << f;
      }
    }
  }
}
