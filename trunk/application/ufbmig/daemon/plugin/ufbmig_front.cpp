#include "ufbmig_front.hpp"
#include "ufbmig_back.hpp"

#include <errno.h>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

#include <cstdio>
#include <cstring>
#include <string>
#include <cstdlib>
#include <iostream>

#include "ServerCommunication.h"
#include "Server.h"
#include "Message.h"
#include "ServerSettings.hpp"

#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

static const std::string SERVER_APP_NAME("ISIM(at)SDPA");
static const std::string SERVER_APP_VERS("0.0.1");

namespace command
{
  enum code
    {
      INITIALIZE = 0,
      MIGRATE = 1,
      MIGRATE_WITH_SALT_MASK = 2,
      ABORT = 3,
      FINALIZE = 4,
    };
}

class UfBMigFrontImpl : FHG_PLUGIN
                      , public ufbmig::Frontend
{
public:
  UfBMigFrontImpl ()
  {
    // start server communication
    PSProMigIF::StartServer::registerInstance
      ( SERVER_APP_NAME
      , new PSProMigIF::ServerHelper(SERVER_APP_VERS)
      );

    m_server = PSProMigIF::StartServer::getInstance(SERVER_APP_NAME);
  }

  FHG_PLUGIN_START()
  {
    std::string def_timeout = fhg_kernel()->get("timeout_default", "-1");
    m_conn_timeout =
      boost::lexical_cast<int>(fhg_kernel()->get("timeout_connect", def_timeout));
    m_send_timeout =
      boost::lexical_cast<int>(fhg_kernel()->get("timeout_send", def_timeout));
    m_recv_timeout =
      boost::lexical_cast<int>(fhg_kernel()->get("timeout_recv", def_timeout));

    PSProMigIF::StartupInfo info;
    info.m_nConnectToTimeout = m_conn_timeout;
    info.m_nWaitForConnectionTimeout = m_conn_timeout;
    info.m_uPort =
      boost::lexical_cast<unsigned int>(fhg_kernel()->get("port_server", "55555"));
    info.m_uDaemonPort =
      boost::lexical_cast<unsigned int>(fhg_kernel()->get("port_daemon", "26698"));
    info.m_uFileEnginePort =
      boost::lexical_cast<unsigned int>(fhg_kernel()->get("port_file_engine", "26699"));
    info.m_uRemotePort = info.m_uPort;
    info.m_uRemoteDaemonPort = info.m_uDaemonPort;
    info.m_uRemoteFileEnginePort = info.m_uFileEnginePort;

    m_server->init(info);
    m_server->handleExceptionsByLibrary(false);
    m_server->addCommunication (new PSProMigIF::ServerCommunicationListen());

    try
    {
      MLOG(INFO, "UfBMig frontend starting on port " << info.m_uPort);

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

    m_message_thread = boost::thread (&UfBMigFrontImpl::message_thread, this);

    fhg_kernel()->acquire<ufbmig::Backend>("ufbmig_back")->registerFrontend(this);

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    m_server->stop(); // FIXME: deadlock!
    m_message_thread.interrupt();
    m_message_thread.join ();

    FHG_PLUGIN_STOPPED();
  }

  int initialize(std::string const &)
  {
    return 0;
  }

  int calculate()
  {
    return 0;
  }

  int finalize()
  {
    return 0;
  }

  int cancel()
  {
    return 0;
  }

  void initialize_done (int)
  {
    return;
  }

  void calculate_done (int)
  {
    return;
  }

  void finalize_done (int)
  {
    return;
  }

  void cancel_done (int)
  {
    return;
  }
private:
  void message_thread ()
  {
    PSProMigIF::Message* msg = 0;

    for (;;)
    {

      try
      {
        boost::this_thread::interruption_point();

        msg = PSProMigIF::Message::recvMsg(m_server->communication(), m_recv_timeout);

        boost::this_thread::interruption_point();

        MLOG(INFO, "got command: " << msg->m_nCommand);
        MLOG(INFO, "custom size: " << msg->m_ulCustomDataSize);
        MLOG(INFO, "custom data: " << std::string(msg->getCostumPtr(), msg->m_ulCustomDataSize));

        switch (msg->m_nCommand)
        {
        case command::INITIALIZE:
          // take user data and write file to disk...
          // initialize with path to that file
          initialize("");
          break;
        case command::MIGRATE:
          // take xml file, write to disk...
          calculate();
          break;
        case command::MIGRATE_WITH_SALT_MASK:
          // take xml file, write to disk...
          // receive salt mask message
          // call calculate
          calculate();
          break;
        case command::ABORT:
          // abort
          cancel();
          break;
        case command::FINALIZE:
          finalize();
          break;
        default:
          break;
        }

        delete msg; msg = 0;
      }
      catch (boost::thread_interrupted const &)
      {
        MLOG(TRACE, "message thread interrupted");
        break;
      }
      catch (PSProMigIF::StartServer::StartServerException const &ex)
      {
        LOG(ERROR, "could not receive message: " << ex.what());
        if (msg) delete msg;
      }
      catch (...)
      {
        MLOG(ERROR, "error during message receive!");
      }
    }

    if (msg) delete msg;
  }

  int m_conn_timeout;
  int m_send_timeout;
  int m_recv_timeout;
  PSProMigIF::StartServer* m_server;
  boost::thread m_message_thread;
};

EXPORT_FHG_PLUGIN( ufbmig_front
                 , UfBMigFrontImpl
                 , "provides the frontend functionality for the UfBMig"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , "ufbmig_back"
                 , ""
                 );
