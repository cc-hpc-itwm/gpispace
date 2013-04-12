// bernd.loerwald@itwm.fraunhofer.de

#ifndef PREFIX_DUMMY_SERVER_HPP
#define PREFIX_DUMMY_SERVER_HPP

#include <QTcpSocket>
#include <QTcpServer>
#include <QThread>
#include <QMutex>
#include <QStringList>

class server : public QTcpServer
{
  Q_OBJECT;
public:
  server (QObject* parent = NULL);

protected:
  virtual void incomingConnection (int);
};

class thread : public QThread
{
  Q_OBJECT;
public:
  thread (int socket_descriptor, QObject* parent = NULL);

protected:
  void run();

private slots:
  void may_read();

  void send_some_status_updates();

private:
  int _socket_descriptor;
  QTcpSocket* _socket;

  mutable QMutex _pending_status_updates_mutex;
  QStringList _pending_status_updates;
};

#endif
