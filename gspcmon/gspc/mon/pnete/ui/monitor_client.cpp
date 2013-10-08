// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/monitor_client.hpp>

#include <fhg/util/num.hpp>
#include <fhg/util/parse/error.hpp>
#include <fhg/util/parse/require.hpp>

#include <boost/bind.hpp>

#include <QColor>

#include <sstream>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace
      {
        namespace require
        {
          using namespace fhg::util::parse::require;

          QString qstring (fhg::util::parse::position& pos)
          {
            return QString::fromStdString (string (pos));
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
            require::list (pos, boost::bind (f, _1, label (pos)));
          }

          void list_of_named_lists
            ( fhg::util::parse::position& pos
            , const boost::function<void (fhg::util::parse::position&, const QString&)>& f
            )
          {
            require::list (pos, boost::bind (named_list, _1, f));
          }
        }
      }

      monitor_client::monitor_client (const QString& host, int port, QObject* parent)
        : QObject (parent)
        , _timer (NULL)
      {
        connect (&_socket, SIGNAL (readyRead()), SLOT (may_read()));
        _socket.connectToHost (host, port);
        if (!_socket.waitForConnected())
        {
          throw std::runtime_error
            (qPrintable ("failed to connect: " + _socket.errorString()));
        }

        QTimer* timer (new QTimer (this));
        connect (timer, SIGNAL (timeout()), SLOT (send_outstanding()));
        timer->start (100);

        resume();
      }

      void monitor_client::pause()
      {
        delete _timer;
        _timer = NULL;
      }
      void monitor_client::resume()
      {
        delete _timer;
        _timer = new QTimer (this);
        connect (_timer, SIGNAL (timeout()), SLOT (check_for_incoming_messages()));
        _timer->start (100);
      }

      void monitor_client::push (const QString& message)
      {
        const QMutexLocker lock (&_outstanding_outgoing_lock);
        _outstanding_outgoing << message;
      }

      void monitor_client::send_outstanding()
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

      void monitor_client::may_read()
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

      void monitor_client::request_possible_status()
      {
        push ("possible_status");
      }

      void monitor_client::request_hostlist()
      {
        push ("hosts");
      }

      void monitor_client::request_status (const QSet<QString> nodes_to_update)
      {
        static const int chunk_size (1000);

        for ( QSet<QString>::const_iterator it (nodes_to_update.constBegin())
                ; it != nodes_to_update.constEnd()
                ;
            )
        {
          QString message ("status: [");
          for (
            ; message.size() < chunk_size && it != nodes_to_update.constEnd()
              ; ++it
              )
          {
            message.append ("\"").append (*it).append ("\",");
          }
          message.append ("]");
          push (message);
        }
      }

      void monitor_client::request_layout_hint (const QString& state)
      {
        push (QString ("layout_hint: [\"%1\"]").arg (state));
      }

      void monitor_client::request_action_description (const QStringList& actions)
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
          push (message);
        }
      }

      void monitor_client::request_action
        ( const QString& hostname
        , const QString& action
        , const QMap<QString, boost::function<QString()> >& value_getters
        )
      {
        std::stringstream ss;
        if (!value_getters.isEmpty())
        {
          ss << ", arguments: [";

          QMap<QString, boost::function<QString()> >::const_iterator i
            (value_getters.constBegin());
          while (i != value_getters.constEnd())
          {
            ss << "\""
               << i.key().toStdString()
               << "\" : \""
               << i.value()()
              .replace ("\\", "\\\\").replace ("\"", "\\\"")
              .toStdString()
               << "\",";
            ++i;
          }

          ss << "]";
        }

        push ( QString ("action: [[host: \"%1\", action: \"%2\"%3]]")
             .arg (hostname)
             .arg (action)
             .arg (QString::fromStdString (ss.str()))
             );
      }

      void monitor_client::possible_status (fhg::util::parse::position& pos)
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

      monitor_client::action_argument_data::action_argument_data (const QString& name)
        : _name (name)
      { }

      void monitor_client::action_argument_data::append (fhg::util::parse::position& pos)
      {
        pos.skip_spaces();

        if (pos.end() || (*pos != 'd' && *pos != 'l' && *pos != 't'))
        {
          throw fhg::util::parse::error::expected
            ("default' or 'label' or 'type", pos);
        }

        switch (*pos)
        {
        case 'd':
          ++pos;
          pos.require ("efault");
          require::token (pos, ":");

          _default = require::qstring (pos);

          break;

        case 'l':
          ++pos;
          pos.require ("abel");
          require::token (pos, ":");

          _label = require::qstring (pos);

          break;

        case 't':
          ++pos;
          pos.require ("ype");
          require::token (pos, ":");
          pos.skip_spaces();

          if ( pos.end() || ( *pos != 'b' && *pos != 'd'
                            && *pos != 'f' && *pos != 'i' && *pos != 's'
                            )
             )
          {
            throw fhg::util::parse::error::expected
              ("boolean' or 'directory' or 'duration' or 'filename' or 'integer' or 'string", pos);
          }

          switch (*pos)
          {
          case 'b':
            ++pos;
            pos.require ("oolean");

            _type = boolean;

            break;

          case 'd':
            ++pos;
            if (pos.end() || (*pos != 'i' && *pos != 'u'))
            {
              throw fhg::util::parse::error::expected ("irectory' or 'uration", pos);
            }

            switch (*pos)
            {
            case 'i':
              ++pos;
              pos.require ("rectory");

              _type = directory;

              break;

            case 'u':
              ++pos;
              pos.require ("ration");

              _type = duration;

              break;
            }

            break;

          case 'f':
            ++pos;
            pos.require ("ilename");

            _type = filename;

            break;

          case 'i':
            ++pos;
            pos.require ("nteger");

            _type = integer;

            break;

          case 's':
            ++pos;
            pos.require ("tring");

            _type = string;

            break;
          }

          break;
        }
      }

      namespace
      {
        void action_argument ( fhg::util::parse::position& pos
                             , QList<monitor_client::action_argument_data>* data_list
                             )
        {
          const QString name (require::qstring (pos));
          require::token (pos, ":");

          monitor_client::action_argument_data data (name);
          require::list (pos, boost::bind (&monitor_client::action_argument_data::append, &data, _1));

          data_list->append (data);
        }
      }

      void monitor_client::action_description
        (fhg::util::parse::position& pos, const QString& action)
      {
        pos.skip_spaces();

        if (pos.end() || (*pos != 'a' && *pos != 'e' && *pos != 'l'))
        {
          throw fhg::util::parse::error::expected
            ("arguments' or 'expected_next_state' or 'long_text", pos);
        }

        switch (*pos)
        {
        case 'a':
          ++pos;
          pos.require ("rguments");
          require::token (pos, ":");

          {
            QList<action_argument_data> data;
            require::list (pos, boost::bind (action_argument, _1, &data));

            emit states_actions_arguments (action, data);
          }

          break;

        case 'e':
          ++pos;
          pos.require ("xpected_next_state");
          require::token (pos, ":");

          emit states_actions_expected_next_state (action, require::qstring (pos));

          break;

        case 'l':
          ++pos;
          pos.require ("ong_text");
          require::token (pos, ":");

          emit states_actions_long_text (action, require::qstring (pos));

          break;
        }
      }

      void monitor_client::layout_hint
        (fhg::util::parse::position& pos, const QString& state)
      {
        pos.skip_spaces();

        if (pos.end() || (*pos != 'b' && *pos != 'c' && *pos != 'h'))
        {
          throw fhg::util::parse::error::expected
            ("border' or 'character' or 'color' or 'hidden", pos);
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

              emit states_layout_hint_character (state, require::character (pos));

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

        case 'h':
          ++pos;
          pos.require ("idden");
          require::token (pos, ":");

          emit states_layout_hint_hidden (state, require::boolean (pos));

          break;
        }
      }

      namespace
      {
        void status_update_data ( fhg::util::parse::position& pos
                                , boost::optional<QString>* details
                                , boost::optional<QString>* state
                                )
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

            *details = require::qstring (pos);

            break;

          case 's':
            ++pos;
            pos.require ("tate");
            require::token (pos, ":");

            *state = require::qstring (pos);

            break;
          }
        }
      }

      void monitor_client::status_update (fhg::util::parse::position& pos)
      {
        const QString host (require::label (pos));
        boost::optional<QString> details (boost::none);
        boost::optional<QString> state (boost::none);
        require::list
          (pos, boost::bind (&status_update_data, _1, &details, &state));
        emit nodes_state (host, state);
        if (details)
        {
          emit nodes_details (host, *details);
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

          boost::optional<monitor_client::action_result_code> _result;
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

                  _result = monitor_client::fail;

                  break;

                case 'o':
                  ++pos;
                  pos.require ("kay");

                  _result = monitor_client::okay;

                  break;

                case 'w':
                  ++pos;
                  pos.require ("arn");

                  _result = monitor_client::warn;

                  break;
                }
              }

              break;
            }
          }
        };
      }

      void monitor_client::action_result (fhg::util::parse::position& pos)
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

      void monitor_client::check_for_incoming_messages()
      {
        messages_type messages;
        {
          const QMutexLocker lock (&_outstanding_incoming_lock);
          std::swap (messages, _outstanding_incoming);
        }

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
              throw fhg::util::parse::error::expected
                ("action_' or 'hosts' or 'layout_hint' or 'possible_status' or 'status", pos);
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
                  , boost::bind (&monitor_client::action_description, this, _1, _2)
                  );

                break;

              case 'r':
                ++pos;
                pos.require ("esult");
                require::token (pos, ":");

                require::list
                  (pos, boost::bind (&monitor_client::action_result, this, _1));
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
                , boost::bind (&monitor_client::layout_hint, this, _1, _2)
                );

              break;

            case 'p':
              ++pos;
              pos.require ("ossible_status");
              require::token (pos, ":");

              require::list
                (pos, boost::bind (&monitor_client::possible_status, this, _1));

              break;

            case 's':
              ++pos;
              pos.require ("tatus");
              require::token (pos, ":");

              require::list
                (pos, boost::bind (&monitor_client::status_update, this, _1));

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
    }
  }
}
