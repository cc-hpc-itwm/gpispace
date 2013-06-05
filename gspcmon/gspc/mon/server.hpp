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
      server ( const QString &workdir
             , int port
             , const QString& hostlist
             , const QDir& hookdir
             , QObject* parent = NULL
             );
      ~server ();

    protected:
      virtual void incomingConnection (int);

    private:
      QDir _workdir;
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
          , color ("0xFFFFFF")
          , border (0)
          , actions ()
        {}

        QString                  name;
        QString                  color;
        boost::optional<QString> label;
        int                      border;
        QStringList              actions;
      };

      void execute_action (fhg::util::parse::position&);
      void send_action_description (fhg::util::parse::position&);
      void send_layout_hint (fhg::util::parse::position&);
      QString description (QString const& action);
      state_info_t & add_state ( QString const &name
                               , QString const &color
                               , int border = 0
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

      // maps from short hook name to full-path + description
      QMap<QString, QPair<QString, QString> > _hooks;

      // maps states to list of possible hooks
      QMap<QString, state_info_t> _states;

      mutable QMutex _pending_status_updates_mutex;
      QStringList _pending_status_updates;
    };
  }
}

#endif
