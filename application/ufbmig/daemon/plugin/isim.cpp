#include "isim.hpp"

#include <list>
#include <boost/thread.hpp>

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

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    if (m_server)
    {
      close (m_server->communication ()->socket ()->getFd ());
    }

    FHG_PLUGIN_STOPPED();
  }

  FHG_ON_PLUGIN_LOADED(plugin) {}

  msg_t *recv (int timeout)
  {
    PSProMigIF::Message *pspro_msg =
      PSProMigIF::Message::recvMsg( m_server->communication()
                                  , timeout
                                  );
    if (pspro_msg)
    {
      msg_t *msg = (msg_t *)malloc (sizeof (msg_t));
      msg->pspro_msg = pspro_msg;
      return msg;
    }
    else
    {
      return 0;
    }
  }

  int send (msg_t **p_msg, int timeout)
  {
    assert (p_msg);

    try
    {
      msg_t *msg = *p_msg;
      msg->pspro_msg->sendMsg ( m_server->communication ()
                              , timeout
                              );
      msg_destroy (p_msg);
    }
    catch (std::exception const &ex)
    {
      msg_destroy (p_msg);
      return -EIO;
    }

    return 0;
  }

  void stop ()
  {
    if (m_server)
    {
      close (m_server->communication ()->socket ()->getFd ());
    }
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
  mutable mutex_type m_mtx_server;

  int m_conn_timeout;
  PSProMigIF::StartServer* m_server;

  std::string m_server_app_name;
  std::string m_server_app_vers;
};

namespace detail
{
    void ISIMServer::startRoutine(void) throw (StartServerException) { }

    void ISIMServer::stopRoutine(void) throw (StartServerException)
    {
      m_isim->stop ();
    }

    void ISIMServer::disconnectRoutine(void) throw (StartServerException) { }

    void ISIMServer::reconnectRoutine(void) throw (StartServerException) { }

    void ISIMServer::idleRoutine(void) throw (StartServerException) { }

    void ISIMServer::busyRoutine(void) throw (StartServerException) { }

    void ISIMServer::crashRoutine(void) throw (StartServerException) { }
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
