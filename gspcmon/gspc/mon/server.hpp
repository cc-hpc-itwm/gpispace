// {petry,bernd.loerwald}@itwm.fraunhofer.de

#ifndef GSPC_MON_SERVER_HPP
#define GSPC_MON_SERVER_HPP

#include <fhg/util/parse/position.hpp>
#include <boost/optional.hpp>

#include <QTcpSocket>
#include <QTcpServer>
#include <QThread>
#include <QMutex>
#include <QStringList>
#include <QDir>

namespace gspc
{
  namespace mon
  {
    class server : public QTcpServer
    {
      Q_OBJECT;
    public:
      server ( int port
             , const QString& hostlist
             , const QDir& hookdir
             , QObject* parent = NULL
             );
      ~server ();

    protected:
      virtual void incomingConnection (int);

    private:
      QString _hostlist;
      QDir _hookdir;
    };

    class thread : public QThread
    {
      Q_OBJECT;
    public:
      thread ( int socket_descriptor
             , const QString& hostlist
             , const QDir& hookdir
             , QObject* parent = NULL);
      ~thread ();

    protected:
      void run();

    private slots:
      void may_read();

      void send_some_status_updates();

      void read_hostlist (const QString&);

    private:
      struct host_state_t
      {
        QString state;
        QString details;
        int     age;
      };

      struct state_info_t
      {
        state_info_t ()
          : name ()
          , color (0)
          , border (0)
          , actions ()
        {}

        QString                  name;
        int color;
        int border;
        boost::optional<QString> label;
        QStringList              actions;
      };

      struct parameter_info_t
      {
        parameter_info_t ()
          : name ()
          , label ()
          , type ()
          , dflt (boost::none)
        {}

        parameter_info_t ( QString const &name
                         , QString const &label
                         , QString const &type
                         , boost::optional<QString> const &dflt = boost::none
                         )
          : name (name)
          , label (label)
          , type (type)
          , dflt (dflt)
        {}

        QString name;
        QString label;
        QString type;
        boost::optional<QString> dflt;
      };

      struct action_info_t
      {
        action_info_t ()
          : name ()
          , path ()
          , desc ()
          , params ()
        {}

        QString name;
        QString path;
        QString desc;
        QList<parameter_info_t> params;
      };

      struct action_result_t
      {
        QString host;
        int exit_code; // 0 = success, 1 = warning, >= 2 = failed
        QString state;
        QString message;
      };

      void execute_action (fhg::util::parse::position&);
      void send_action_description (fhg::util::parse::position&);
      void send_layout_hint (fhg::util::parse::position&);
      QString description (QString const& action);
      state_info_t & add_state ( QString const &name
                               , int color
                               , int border = 0
                               );

      action_info_t & add_action ( QString const &name
                                 , QString const &desc
                                 );

      int call_action ( thread::action_info_t const &ai
                      , QMap<QString, QString> const &params
                      , QStringList const &nodes
                      , QMap<QString, action_result_t> &result
                      , QString &error_reason
                      );

      void send_status_updates (QStringList const &hosts);
      void send_status ( QString const &host
                       , QString const &state
                       , QString const &detail
                       );
      void update_status (QStringList const&hosts);

      int _socket_descriptor;
      QTcpSocket* _socket;
      QDir _hookdir;

      mutable QMutex _hosts_mutex;
      QMap<QString, host_state_t> _hosts;

      QMap<QString, action_info_t> _actions;
      // HACK: cache the workdir -> will be set when calling 'start'
      QString _workdir;

      QMap<QString, state_info_t> _states;

      mutable QMutex _pending_status_updates_mutex;
      QStringList _pending_status_updates;
    };
  }
}

#endif
