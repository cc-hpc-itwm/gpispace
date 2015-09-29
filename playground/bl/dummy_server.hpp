// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <fhg/util/parse/position.hpp>

#include <QTcpSocket>
#include <QTcpServer>
#include <QThread>
#include <QMutex>
#include <QStringList>

class server : public QTcpServer
{
  Q_OBJECT
public:
  server (int port, const QString& hostlist, QObject* parent = nullptr);

protected:
  virtual void incomingConnection (int) override;

private:
  QString _hostlist;
};

class thread : public QThread
{
  Q_OBJECT
public:
  thread (int socket_descriptor, const QString& hostlist, QObject* parent = nullptr);

protected:
  virtual void run() override;

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

  mutable QMutex _hosts_mutex;
  QMap<QString, QPair<QString, int /*ticks since last change*/>> _hosts;

  mutable QMutex _pending_status_updates_mutex;
  QStringList _pending_status_updates;
};
