// {petry,bernd.loerwald}@itwm.fraunhofer.de

#ifndef GSPC_MON_SERVER_HPP
#define GSPC_MON_SERVER_HPP

#include <fhg/util/parse/position.hpp>

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
      void execute_action (fhg::util::parse::position&);
      void send_action_description (fhg::util::parse::position&);
      void send_layout_hint (fhg::util::parse::position&);

      int _socket_descriptor;
      QTcpSocket* _socket;
      QDir _hookdir;

      mutable QMutex _hosts_mutex;
      QMap<QString, QPair<QString, int /*ticks since last change*/> > _hosts;

      mutable QMutex _pending_status_updates_mutex;
      QStringList _pending_status_updates;
    };
  }
}

#endif
