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

static const std::string SERVER_APP_NAME("Interactive Migration");
static const std::string SERVER_APP_VERS("1.0.0");

namespace client { namespace command {
    // sent by the GUI to us
    enum code
      {
        INITIALIZE = 0,
        MIGRATE,
        MIGRATE_WITH_SALT_MASK,
        SALT_MASK,
        ABORT,
        FINALIZE,
      };
  }
}

namespace server { namespace command {
    // we might send those
    enum code
      {
        WAITING_FOR_INITIALIZE = 0,

        INITIALIZING = 1,
        INITIALIZE_SUCCESS = 2,
        INITIALIZE_FAILURE = 3,

        MIGRATING = 4,
        MIGRATE_SUCCESS = 5,
        MIGRATE_FAILURE = 6,

        FINALIZING = 7,
        FINALIZE_SUCCESS = 8,
        FINALIZE_FAILURE = 9,

        ABORT_ACCEPTED = 10,
        ABORT_REFUSED = 11,

        PROGRESS = 1000,
        LOGOUTPUT = 1001,
      };
  }
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
    m_migrate_xml_buffer.clear();

    std::string def_timeout = fhg_kernel()->get("timeout_default", "-1");
    m_conn_timeout =
      boost::lexical_cast<int>(fhg_kernel()->get("timeout_connect", def_timeout));
    m_send_timeout =
      boost::lexical_cast<int>(fhg_kernel()->get("timeout_send", def_timeout));
    m_recv_timeout =
      boost::lexical_cast<int>(fhg_kernel()->get("timeout_recv", def_timeout));

    MLOG( TRACE
        ,  "timeouts: connect = " << m_conn_timeout
        << " send = " << m_send_timeout
        << " recv = " << m_recv_timeout
        );

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

    try
    {
      m_server->handleExceptionsByLibrary(false);
      m_server->init(info);
      m_server->addCommunication (new PSProMigIF::ServerCommunicationListen());

      MLOG(TRACE, "UfBMig frontend starting on port " << info.m_uPort);

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
    catch (...)
    {
      LOG(ERROR, "could not start server connection due to an unknown reason");
      FHG_PLUGIN_FAILED(EFAULT);
    }

    m_backend = fhg_kernel()->acquire<ufbmig::Backend>("ufbmig_back");
    assert (m_backend);
    m_backend->registerFrontend(this);

    m_stop_requested = false;
    m_message_thread = boost::thread (&UfBMigFrontImpl::message_thread, this);

    send_waiting_for_initialize();

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    //m_server->stop(); // FIXME: deadlock!
    m_stop_requested = true;
    m_message_thread.interrupt();
    m_message_thread.join ();

    FHG_PLUGIN_STOPPED();
  }

  int initialize(std::string const &s)
  {
    assert (m_backend);
    return m_backend->initialize (s);
  }

  int update_salt_mask(const char *data, size_t len)
  {
    return m_backend->update_salt_mask(data, len);
  }

  int calculate(std::string const &xml)
  {
    assert (m_backend);
    return m_backend->calculate (xml);
  }

  int finalize()
  {
    assert (m_backend);
    return m_backend->finalize ();
  }

  int cancel()
  {
    assert (m_backend);
    return m_backend->cancel();
  }

  void initialize_done (int ec)
  {
    LOG(TRACE, "initialize done: " << ec);
    if (! ec)
    {
      send_initialize_success();
    }
    else
    {
      send_initialize_failure(ec);
    }
  }

  void calculate_done (int ec)
  {
    LOG(TRACE, "calculate done: " << ec);
    if (! ec)
    {
      // get current output from backend
      const char *data = 0;
      const size_t len = 0;

      send_migrate_success(data, len);
    }
    else
    {
      send_migrate_failure(ec);
    }
  }

  void finalize_done (int ec)
  {
    LOG(TRACE, "finalize done: " << ec);
    if (! ec)
    {
      send_finalize_success();
    }
    else
    {
      send_finalize_failure(ec);
    }
  }
private:
  boost::shared_ptr<PSProMigIF::Message> create_pspro_message( int cmd
                                                             , const void *data = 0
                                                             , size_t len = 0
                                                             )
  {
    assert ((len && data != NULL) || !len);

    boost::shared_ptr<PSProMigIF::Message> msg
      (PSProMigIF::Message::generateMsg(len));
    msg->m_nCommand = cmd;
    memcpy( msg->getCostumPtr(), data, len);
    return msg;
  }

  void send_waiting_for_initialize()
  {
    m_server->idle();
    create_pspro_message(server::command::WAITING_FOR_INITIALIZE)
      ->sendMsg(m_server->communication());
  }

  void send_initializing()
  {
    m_server->busy();
    create_pspro_message(server::command::INITIALIZING)
      ->sendMsg(m_server->communication());
  }

  void send_initialize_success()
  {
    m_server->idle();
    create_pspro_message(server::command::INITIALIZE_SUCCESS)
      ->sendMsg(m_server->communication());
  }

  void send_initialize_failure(int ec)
  {
    m_server->idle();
    create_pspro_message(server::command::INITIALIZE_FAILURE, &ec, sizeof(ec))
      ->sendMsg(m_server->communication());
  }

  void send_migrating()
  {
    m_server->busy();
    create_pspro_message(server::command::MIGRATING)
      ->sendMsg(m_server->communication());
  }

  void send_migrate_success(const void *data, size_t len)
  {
    m_server->idle();
    create_pspro_message(server::command::MIGRATE_SUCCESS, data, len)
      ->sendMsg(m_server->communication());
  }

  void send_migrate_failure(int ec)
  {
    m_server->idle();
    create_pspro_message(server::command::MIGRATE_FAILURE, &ec, sizeof(ec))
      ->sendMsg(m_server->communication());
  }

  void send_finalizing()
  {
    m_server->busy();
    create_pspro_message(server::command::FINALIZING)
      ->sendMsg(m_server->communication());
  }

  void send_finalize_success()
  {
    m_server->idle();
    create_pspro_message(server::command::FINALIZE_SUCCESS)
      ->sendMsg(m_server->communication());
  }

  void send_finalize_failure(int ec)
  {
    m_server->idle();
    create_pspro_message(server::command::FINALIZE_FAILURE, &ec, sizeof(ec))
      ->sendMsg(m_server->communication());
  }

  void send_progress(int p)
  {
    create_pspro_message(server::command::PROGRESS, &p, sizeof(p))
      ->sendMsg(m_server->communication());
  }

  void send_logoutput(std::string const &msg)
  {
    create_pspro_message(server::command::LOGOUTPUT, msg.c_str(), msg.size())
      ->sendMsg(m_server->communication());
  }

  void send_abort_accepted()
  {
    create_pspro_message(server::command::ABORT_ACCEPTED)
      ->sendMsg(m_server->communication());
  }

  void send_abort_refused()
  {
    create_pspro_message(server::command::ABORT_REFUSED)
      ->sendMsg(m_server->communication());
  }

  void message_thread ()
  {
    typedef boost::shared_ptr<PSProMigIF::Message> message_ptr;

    MLOG(TRACE, "waiting for messages...");

    while (!m_stop_requested)
    {
      try
      {
        int ec;
        std::string payload;

        message_ptr msg
          (PSProMigIF::Message::recvMsg(m_server->communication(), m_recv_timeout));
        if (! msg)
        {
          continue;
        }

        if (msg->m_ulCustomDataSize)
        {
          payload = std::string(msg->getCostumPtr(), msg->m_ulCustomDataSize);
        }

        ec = handle_ISIM_command(msg->m_nCommand, payload);
      }
      catch (PSProMigIF::StartServer::StartServerException const &ex)
      {
        LOG(ERROR, "could not receive message: " << ex.what());
      }
      catch (...)
      {
        MLOG(ERROR, "error during message receive!");
      }
    }
  }

  int handle_ISIM_command(int cmd, std::string const & payload)
  {
    int ec = 0;

    MLOG(INFO, "got command: " << cmd);
    MLOG(INFO, "custom size: " << payload.size());
    //    MLOG(INFO, "custom data: " << payload);

    switch (cmd)
    {
    case client::command::INITIALIZE:
      ec = initialize(payload);
      if (0 == ec)
      {
        send_initializing();
      }
      else
      {
        send_initialize_failure(ec);
      }
      break;
    case client::command::MIGRATE:
      ec = calculate(payload);
      if (0 == ec)
      {
        send_migrating();
      }
      else
      {
        send_migrate_failure(ec);
      }
      break;
    case client::command::MIGRATE_WITH_SALT_MASK:
      // just remember xml data, asume we get the salt mask afterwards
      m_migrate_xml_buffer = payload;
      break;
    case client::command::SALT_MASK:
      ec = update_salt_mask(0, 0);
      // work around strange protocol
      if (! m_migrate_xml_buffer.empty())
      {
        ec = handle_ISIM_command (client::command::MIGRATE, m_migrate_xml_buffer);
        m_migrate_xml_buffer.clear();
      }
      break;
    case client::command::ABORT:
      // abort
      if (0 == cancel())
      {
        send_abort_accepted();
      }
      else
      {
        send_abort_refused();
      }
      break;
    case client::command::FINALIZE:
      ec = finalize();
      if (0 == ec)
      {
        send_finalizing();
      }
      else
      {
        send_finalize_failure(ec);
      }
      break;
    default:
      break;
    }

    return ec;
  }

  bool m_stop_requested;
  int m_conn_timeout;
  int m_send_timeout;
  int m_recv_timeout;
  PSProMigIF::StartServer* m_server;
  ufbmig::Backend *m_backend;
  boost::thread m_message_thread;

  std::string m_migrate_xml_buffer;
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
