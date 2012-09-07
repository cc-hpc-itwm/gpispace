#include "isim.hpp"

#include <list>
#include <boost/thread.hpp>

#include <sys/types.h>    // setsockopt
#include <sys/socket.h>   // setsockopt

#include <stdio.h> // broken includes in pspro*

#include "ServerCommunication.h"
#include "Server.h"
#include "Message.h"
#include "ServerSettings.hpp"
#include "ServerStateControlServer.h"

#include <fhg/assert.hpp>
#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

#include <stdexcept>

static void unexpected_handler ()
{
        MLOG (ERROR, "something really really bad happened");
}

namespace isim
{
  struct _msg_t
  {
    PSProMigIF::Message *pspro_msg;
    int timeout;
  };
}

using namespace isim;

typedef boost::recursive_mutex mutex_type;
typedef boost::unique_lock<mutex_type> lock_type;
typedef boost::condition_variable_any condition_type;

class ISIM_Real;
namespace detail
{
  using namespace PSProMigIF;

  class ISIMServer;

  class ISIMServerHelper : public StartServerHelper
  {
  public:
    ISIMServerHelper (std::string const &vsn, ISIM_Real *isim)
      : StartServerHelper (vsn)
      , m_isim (isim)
    {}

    StartServer *create ();

    ServerStateControl *createStateControl ()
    {
      return new ServerStateControlServer ();
    }
  private:
    ISIM_Real *m_isim;
  };

  class ISIMServer : public StartServer
  {
    friend class ISIMServerHelper;

  protected:
    ISIMServer (const std::string& vsn, ISIM_Real *isim)
      : StartServer (vsn)
      , m_isim (isim)
    {}

    virtual ~ISIMServer ()
    {}

    void startRoutine(void) throw (StartServerException);
    void stopRoutine(void) throw (StartServerException);
    void disconnectRoutine(void) throw (StartServerException);
    void reconnectRoutine(void) throw (StartServerException);
    void idleRoutine(void) throw (StartServerException);
    void busyRoutine(void) throw (StartServerException);
    void crashRoutine(void) throw (StartServerException);
  private:
    ISIM_Real *m_isim;
  };

  StartServer *ISIMServerHelper::create ()
  {
    return new ISIMServer (version (), m_isim);
  }
}

class ISIM_Real : FHG_PLUGIN
                , public isim::ISIM
{
  typedef std::list <msg_t *> msg_queue_t;
public:
  ISIM_Real () {}
  ~ISIM_Real () {}

  FHG_PLUGIN_START()
  {
    std::set_unexpected (&unexpected_handler);

    m_server_app_name =
      fhg_kernel ()->get ("app_name", "Interactive Migration");
    m_server_app_vers =
      fhg_kernel ()->get ("app_vers", "1.0.0");

    m_conn_timeout = fhg_kernel()->get<int>("conn_timeout", -1);

    PSProMigIF::StartupInfo info;
    info.m_nConnectToTimeout = m_conn_timeout;
    info.m_nWaitForConnectionTimeout = m_conn_timeout;
    info.m_uPort =
      fhg_kernel()->get<unsigned short>("port_server", "55555");
    info.m_uDaemonPort =
      fhg_kernel()->get<unsigned short>("port_daemon", "26698");
    info.m_uFileEnginePort =
      fhg_kernel()->get<unsigned short>("port_file_engine", "26699");
    info.m_uRemotePort = info.m_uPort;
    info.m_uRemoteDaemonPort = info.m_uDaemonPort;
    info.m_uRemoteFileEnginePort = info.m_uFileEnginePort;

    try
    {
      // start server communication
      PSProMigIF::StartServer::registerInstance
        ( m_server_app_name
        , new detail::ISIMServerHelper (m_server_app_vers, this)
        );

      m_server = PSProMigIF::StartServer::getInstance(m_server_app_name);
      m_server->handleExceptionsByLibrary(false);
      m_server->init(info);
      m_server->addCommunication (new PSProMigIF::ServerCommunicationListen);

      //MLOG(TRACE, "UfBMig frontend starting on port " << info.m_uPort);

      // start server control object
      m_server->start(false); // false == do not kill running apps

      MLOG(INFO, "UfBMig frontend running on port " << info.m_uPort);
    }
    catch (PSProMigIF::StartServer::StartServerException const &ex)
    {
      LOG(ERROR, "could not start server connection: " << ex.what());
      FHG_PLUGIN_FAILED(ETIMEDOUT);
      // TODO:
      //   mark plugin as incomplete, try to start connection again...
    }
    catch (PSProMigIF::ServerStateControlError)
    {
      LOG(ERROR, "could not start server connection due to an unknown reason");
      FHG_PLUGIN_FAILED(EFAULT);
    }
    catch (...)
    {
      LOG(ERROR, "could not start server connection due to an unknown reason");
      FHG_PLUGIN_FAILED(EFAULT);
    }

    {
      int sock_fd = m_server->communication ()->socket ()->getFd ();
      struct linger linger_value = { 0, 0 };
      setsockopt (sock_fd, SOL_SOCKET, SO_LINGER, &linger_value, sizeof (struct linger));
    }

    m_stop_requested = false;
    m_recv_thread = boost::thread (&ISIM_Real::recv_thread, this);
    m_send_thread = boost::thread (&ISIM_Real::send_thread, this);

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    this->stop ();

    FHG_PLUGIN_STOPPED();
  }

  FHG_ON_PLUGIN_LOADED(plugin) {}

  msg_t *recv (int timeout)
  {
    msg_t *msg = 0;
    if (timeout == -1) // wait until all eternity
    {
      lock_type lck (m_msg_i_q_mtx);
      while (m_msg_i_q.empty ())
      {
        m_msg_i_avail.wait (lck);
      }

      if (not m_msg_i_q.empty ()) // stop requested
      {
        msg = m_msg_i_q.front (); m_msg_i_q.pop_front ();
      }
    }
    else if (timeout == 0) // probe
    {
      lock_type lck (m_msg_i_q_mtx);
      if (not m_msg_i_q.empty ())
      {
        msg = m_msg_i_q.front (); m_msg_i_q.pop_front ();
      }
    }
    else
    {
      lock_type lck (m_msg_i_q_mtx);
      boost::system_time const wait_until =
        boost::get_system_time()
        + boost::posix_time::milliseconds (timeout);
      if (m_msg_i_avail.timed_wait (lck, wait_until))
      {
        if (not m_msg_i_q.empty ()) // stop requested
        {
          msg = m_msg_i_q.front (); m_msg_i_q.pop_front ();
        }
      }
    }

    return msg;
  }

  int send (msg_t **p_msg, int timeout)
  {
    assert (p_msg);

    if (m_stop_requested)
    {
      msg_destroy (p_msg);
      return -EIO;
    }
    else
    {
      lock_type lck (m_msg_o_q_mtx);

      msg_t *msg = *p_msg;
      msg->timeout = timeout;

      m_msg_o_q.push_back (msg);
      *p_msg = 0;
      return 0;
    }
  }

  void stop ()
  {
    m_stop_requested = true;

    if (m_server)
    {
      close (m_server->communication ()->socket ()->getFd ());
    }

    // notify any waiting processes
    m_msg_i_avail.notify_all ();
    m_msg_i_avail.notify_all ();

    m_send_thread.interrupt ();
    m_recv_thread.interrupt ();

    m_send_thread.join ();
    m_recv_thread.join ();
  }

  void idle ()
  {
    assert (m_server);

    m_server->idle ();
  }

  void busy ()
  {
    assert (m_server);

    m_server->busy ();
  }

  msg_t *msg_new (int type, size_t size)
  {
    msg_t *msg = (msg_t*) (malloc (sizeof (msg_t)));
    assert (msg);

    msg->timeout = -1;
    msg->pspro_msg  = PSProMigIF::Message::generateMsg (size);
    assert (msg->pspro_msg);

    msg->pspro_msg->m_nCommand = type;
    return msg;
  }

  void msg_destroy (msg_t **p_msg)
  {
    assert (p_msg);
    if (*p_msg)
    {
      msg_t *msg = *p_msg;
      free (msg->pspro_msg);
      free (msg);
      *p_msg = 0;
    }
  }

  int msg_type (msg_t *msg)
  {
    assert (msg);
    return msg->pspro_msg->m_nCommand;
  }

  void *msg_data (msg_t *msg)
  {
    assert (msg);
    return msg->pspro_msg->getCostumPtr ();
  }

  size_t msg_size (msg_t *msg)
  {
    assert (msg);
    return msg->pspro_msg->m_ulCustomDataSize;
  }
private:
  void recv_thread ()
  {
    while (! m_stop_requested)
    {
      PSProMigIF::Message *pspro_msg =
        PSProMigIF::Message::recvMsg( m_server->communication()
                                    , -1
                                    );
      if (pspro_msg)
      {
        msg_t *msg = (msg_t *)malloc (sizeof (msg_t));
        msg->pspro_msg = pspro_msg;

        lock_type (m_msg_i_q_mtx);
        m_msg_i_q.push_back (msg);
        m_msg_i_avail.notify_one ();
      }
      else
      {
        lock_type (m_msg_i_q_mtx);
        m_msg_i_avail.notify_all ();
        break;
      }
    }
  }

  void send_thread ()
  {
    msg_t *msg;

    lock_type lck(m_msg_o_q_mtx);

    while (! m_stop_requested)
    {
      while (m_msg_o_q.empty ())
      {
        m_msg_o_avail.wait (lck);
      }

      if (not m_stop_requested)
      {
        try
        {
          msg->pspro_msg->sendMsg ( m_server->communication ()
                                  , msg->timeout
                                  );
          msg_destroy (&msg);
        }
        catch (std::exception const &ex)
        {
          msg_destroy (&msg);
        }
      }
    }

    while (not m_msg_o_q.empty ())
    {
      msg = m_msg_o_q.front (); m_msg_o_q.pop_front ();
      delete msg;
    }
  }

  mutable mutex_type m_mtx_server;

  int m_conn_timeout;
  PSProMigIF::StartServer* m_server;

  std::string m_server_app_name;
  std::string m_server_app_vers;

  bool                   m_stop_requested;

  mutable mutex_type     m_msg_i_q_mtx;
  msg_queue_t            m_msg_i_q;
  boost::thread          m_recv_thread;
  mutable condition_type m_msg_i_avail;

  mutable mutex_type     m_msg_o_q_mtx;
  msg_queue_t            m_msg_o_q;
  boost::thread          m_send_thread;
  mutable condition_type m_msg_o_avail;
};

namespace detail
{
    void ISIMServer::startRoutine(void) throw (StartServerException) { }

    void ISIMServer::stopRoutine(void) throw (StartServerException)
    {
      MLOG (INFO, "got stop request");
      m_isim->stop ();
    }

    void ISIMServer::disconnectRoutine(void) throw (StartServerException) {}

    void ISIMServer::reconnectRoutine(void) throw (StartServerException) {}

    void ISIMServer::idleRoutine(void) throw (StartServerException) {}

    void ISIMServer::busyRoutine(void) throw (StartServerException) {}

    void ISIMServer::crashRoutine(void) throw (StartServerException) {}
}

EXPORT_FHG_PLUGIN( isim
                 , ISIM_Real
                 , ""
                 , "provides dummy network functionality with the ISIM GUI"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
