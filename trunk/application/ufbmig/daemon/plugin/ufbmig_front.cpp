#include "ufbmig_front.hpp"
#include "ufbmig_back.hpp"

#include <errno.h>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/util/thread/queue.hpp>

#include <cstdio>
#include <cstring>
#include <string>
#include <cstdlib>
#include <iostream>
#include <functional>

#include "ServerCommunication.h"
#include "Server.h"
#include "Message.h"
#include "ServerSettings.hpp"

#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

static const std::string SERVER_APP_NAME("Interactive Migration");
static const std::string SERVER_APP_VERS("1.0.0");

typedef boost::recursive_mutex mutex_type;
typedef boost::unique_lock<mutex_type> lock_type;
typedef boost::condition_variable_any condition_type;

typedef boost::shared_ptr<PSProMigIF::Message> PSProMessagePtr;

static void my_free(void *ptr)
{
  free(ptr);
}

namespace client { namespace command {
    // sent by the GUI to us
    enum code
      {
        INITIALIZE             = 0,
        MIGRATE                = 1,
        MIGRATE_WITH_SALT_MASK = 2,
        SALT_MASK              = 3,
        ABORT                  = 4,
        FINALIZE               = 5,

        NUM_COMMANDS
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

        PROCESSING_SALT_MASK = 10,
        PROCESS_SALT_MASK_SUCCESS = 11,
        PROCESS_SALT_MASK_FAILURE = 12,

        ABORT_ACCEPTED = 13,
        ABORT_REFUSED = 14,

        MIGRATE_META_DATA = 15,
        MIGRATE_DATA = 16,

        PROGRESS = 1000,
        LOGOUTPUT = 1001,
      };
  }
}

namespace transfer
{
  enum
    {
      PENDING = 0,
      RUNNING = 1,
      CANCELED = 2,
      FINISHED = 3,
    };

  struct request_t
  {
    request_t ()
      : m_state(0)
    {}

    int state() const { lock_type lck(m_mtx); return m_state; }
    void set_state(int s) { lock_type lck(m_mtx); m_state = s; }
  private:
    mutable mutex_type m_mtx;

    int m_state; // 0 pending 1 executing 2 canceled
  };

  typedef boost::shared_ptr<request_t> request_ptr_t;
  typedef fhg::thread::queue<request_ptr_t, std::list> request_queue_t;
}

class UfBMigFrontImpl : FHG_PLUGIN
                      , public ufbmig::Frontend
{
public:
  UfBMigFrontImpl ()
  {
  }

  FHG_PLUGIN_START()
  {
    m_migrate_xml_buffer.clear();

    std::string def_timeout = fhg_kernel()->get("timeout_default", "-1");
    m_conn_timeout = fhg_kernel()->get<int>("timeout_connect", def_timeout);
    m_send_timeout = fhg_kernel()->get<int>("timeout_send", def_timeout);
    m_recv_timeout = fhg_kernel()->get<int>("timeout_recv", def_timeout);

    MLOG( TRACE
        ,  "timeouts: connect = " << m_conn_timeout
        << " send = " << m_send_timeout
        << " recv = " << m_recv_timeout
        );

    m_chunk_size = fhg_kernel()->get<std::size_t>("chunk_size", "4194304");
    DMLOG(TRACE, "using chunk size for GUI messages of: " << m_chunk_size);

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
        ( SERVER_APP_NAME
        , new PSProMigIF::ServerHelper(SERVER_APP_VERS)
      );

      m_server = PSProMigIF::StartServer::getInstance(SERVER_APP_NAME);
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

    m_current_transfer = transfer::request_ptr_t(new transfer::request_t());
    m_current_transfer->set_state(transfer::CANCELED);

    m_backend = fhg_kernel()->acquire<ufbmig::Backend>("ufbmig_back");
    assert (m_backend);
    m_backend->registerFrontend(this);

    m_stop_requested = false;
    m_transfer_thread = boost::thread (&UfBMigFrontImpl::transfer_thread, this);
    m_message_thread = boost::thread (&UfBMigFrontImpl::message_thread, this);

    send_waiting_for_initialize();

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    //m_server->stop(); // FIXME: deadlock!
    m_stop_requested = true;

    m_transfer_thread.interrupt();
    m_transfer_thread.join ();

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
    assert (m_backend);
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
    /*
    m_transfer_requests.clear();
    m_current_transfer->set_state(transfer::CANCELED);
    */
    return m_backend->cancel();
  }

  void initialize_done (int ec)
  {
    if (! ec)
    {
      LOG(TRACE, "initialize done");
      send_initialize_success();
    }
    else
    {
      LOG(WARN, "initialize failed: " << ec);
      send_initialize_failure(ec);
    }
  }

  void salt_mask_done (int ec)
  {
    if (0 == ec)
    {
      LOG(TRACE, "salt mask done");
      send_process_salt_mask_success();

      // work around strange protocol
      if (! m_migrate_xml_buffer.empty())
      {
        const std::string payload (m_migrate_xml_buffer);
        m_migrate_xml_buffer.clear();
        handle_ISIM_command (client::command::MIGRATE, payload);
      }
    }
    else
    {
      LOG(WARN, "salt mask failed: " << ec);
      send_process_salt_mask_failure(ec);

      // work around strange protocol
      if (! m_migrate_xml_buffer.empty())
      {
        m_migrate_xml_buffer.clear();
      }
    }
  }

  void calculate_done (int ec)
  {
    if (! ec)
    {
      LOG(TRACE, "migration done");
      // get current output from backend
      const char *data = 0;
      const size_t len = 0;

      // TODO:
      //    abort running transfers
      //    schedule new data transfer for output
      //    when finished send_migrate_success()

      send_migrate_success();
    }
    else
    {
      LOG(WARN, "migration failed: " << ec);
      send_migrate_failure(ec);
    }
  }

  void finalize_done (int ec)
  {
    if (! ec)
    {
      LOG(TRACE, "finalize done");
      send_finalize_success();
    }
    else
    {
      LOG(WARN, "finalize failed: " << ec);
      send_finalize_failure(ec);
    }
  }

  void update_progress (int value)
  {
    send_progress(value);
  }
private:
  PSProMessagePtr create_pspro_error_message ( int cmd
                                             , int ec
                                             )
  {
    if (ec < 0)
      ec = -ec;

    std::string error (strerror(ec));
    return create_pspro_message(cmd, error.c_str(), error.size());
  }

  PSProMessagePtr create_pspro_message( int cmd
                                      , const void *data = 0
                                      , size_t len = 0
                                      )
  {
    assert ((len && data != NULL) || !len);

    PSProMessagePtr msg(create_new_pspro_message(cmd, len));
    memcpy( msg->getCostumPtr(), data, len);
    return msg;
  }

  PSProMessagePtr create_new_pspro_message (int cmd, size_t len)
  {
    PSProMessagePtr msg
      (PSProMigIF::Message::generateMsg(len), std::ptr_fun(my_free));
    msg->m_nCommand = cmd;
    return msg;
  }

  int send_waiting_for_initialize()
  {
    m_server->idle();
    return send_to_gui(create_pspro_message(server::command::WAITING_FOR_INITIALIZE));
  }

  int send_initializing()
  {
    m_server->busy();
    return send_to_gui(create_pspro_message(server::command::INITIALIZING));
  }

  int send_initialize_success()
  {
    m_server->idle();
    return send_to_gui(create_pspro_message(server::command::INITIALIZE_SUCCESS));
  }

  int send_initialize_failure(int ec)
  {
    m_server->idle();
    return send_to_gui(create_pspro_error_message( server::command::INITIALIZE_FAILURE
                                                 , ec
                                                 )
                      );
  }

  int send_processing_salt_mask()
  {
    m_server->busy();
    return send_to_gui(create_pspro_message(server::command::PROCESSING_SALT_MASK));
  }

  int send_process_salt_mask_success()
  {
    m_server->idle();
    return send_to_gui(create_pspro_message(server::command::PROCESS_SALT_MASK_SUCCESS));
  }

  int send_process_salt_mask_failure(int ec)
  {
    m_server->idle();
    return send_to_gui(create_pspro_error_message( server::command::PROCESS_SALT_MASK_FAILURE
                                                 , ec
                                                 )
                      );
  }

  int send_migrating()
  {
    m_server->busy();
    return send_to_gui(create_pspro_message(server::command::MIGRATING));
  }

  int send_migrate_success()
  {
    m_server->idle();
    int ec = send_to_gui (create_pspro_message(server::command::MIGRATE_SUCCESS));
    if (0 == ec)
    {
      m_transfer_requests.clear ();
      m_current_transfer->set_state(transfer::CANCELED);
      m_transfer_requests.put(transfer::request_ptr_t(new transfer::request_t()));
    }
    return ec;
  }

  int send_migrate_failure(int ec)
  {
    m_server->idle();
    return send_to_gui(create_pspro_error_message(server::command::MIGRATE_FAILURE, ec));
  }

  int send_finalizing()
  {
    m_server->busy();
    return send_to_gui(create_pspro_message(server::command::FINALIZING));
  }

  int send_finalize_success()
  {
    m_server->idle();
    int ec;
    ec = send_to_gui(create_pspro_message(server::command::FINALIZE_SUCCESS));
    ec = send_waiting_for_initialize();
    return ec;
  }

  int send_finalize_failure(int err)
  {
    m_server->idle();
    int ec;
    ec = send_to_gui(create_pspro_error_message(server::command::FINALIZE_FAILURE, err));
    ec = send_waiting_for_initialize();
    return ec;
  }

  int send_progress(int p)
  {
    return send_to_gui(create_pspro_message(server::command::PROGRESS, &p, sizeof(p)));
  }

  int send_logoutput(std::string const &msg)
  {
    return send_to_gui(create_pspro_message(server::command::LOGOUTPUT, msg.c_str(), msg.size()));
  }

  int send_abort_accepted()
  {
    return send_to_gui(create_pspro_message(server::command::ABORT_ACCEPTED));
  }

  int send_abort_refused(int ec)
  {
    return send_to_gui(create_pspro_error_message(server::command::ABORT_REFUSED, ec));
  }

  void message_thread ()
  {
    typedef PSProMessagePtr message_ptr;

    MLOG(TRACE, "waiting for messages...");

    size_t fail_counter = 0;
    while (!m_stop_requested && fail_counter < 100)
    {
      try
      {
        int ec;
        std::string payload;

        message_ptr msg
          ( PSProMigIF::Message::recvMsg(m_server->communication(), m_recv_timeout)
          , std::ptr_fun(my_free)
          );
        if (! msg)
        {
          ++fail_counter;
          continue;
        }
        else
        {
          fail_counter = 0;
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

    if (fail_counter)
    {
      int ec = errno;
      MLOG(ERROR, "terminating, since there were " << fail_counter << " failures while receiving messages from the GUI: " << strerror(ec));
      fhg_kernel()->terminate();
    }
  }

  int handle_ISIM_command(int cmd, std::string const & payload)
  {
    int ec = 0;

    DMLOG(TRACE, "got command: " << cmd);
    DMLOG(TRACE, "custom size: " << payload.size());

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
      // work around protocol fuck up: just  remember xml data, asume we get the
      // salt mask afterwards
      m_migrate_xml_buffer = payload;
      break;
    case client::command::SALT_MASK:
      ec = update_salt_mask(payload.c_str(), payload.size());
      if (0 == ec)
      {
        send_processing_salt_mask();
      }
      else
      {
        send_process_salt_mask_failure(ec);
      }
      break;
    case client::command::ABORT:
      // abort
      ec = cancel ();
      if (0 == ec)
      {
        send_abort_accepted();
      }
      else
      {
        send_abort_refused(ec);
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
      MLOG(ERROR, "Ignoring invalid command from client: " << cmd);
      break;
    }

    return ec;
  }

  void transfer_thread ()
  {
    while (not m_stop_requested)
    {
      int ec = 0;

      m_current_transfer = m_transfer_requests.get();

      if (m_current_transfer->state() != transfer::PENDING)
      {
        MLOG(INFO, "ignoring non pending transfer request!");
        continue;
      }

      m_current_transfer->set_state(transfer::RUNNING);

      MLOG(INFO, "transfering data to GUI");

      // transfer meta data
      ec = transfer_output_meta_data_to_gui(m_current_transfer);
      if (0 != ec)
      {
        MLOG(WARN, "Could not transfer output meta data to GUI: " << strerror(-ec));
        continue;
      }

      ec = transfer_output_data_to_gui(m_current_transfer);
      if (0 != ec)
      {
        MLOG(WARN, "Could not transfer output to GUI: " << strerror(-ec));
        continue;
      }

      m_current_transfer->set_state(transfer::FINISHED);

      MLOG(INFO, "transfer complete");
    }
  }

  int transfer_output_meta_data_to_gui (transfer::request_ptr_t request)
  {
    if (request->state() == transfer::CANCELED)
    {
      return -ECANCELED;
    }

    int fd = m_backend->open("data.output_meta");
    if (fd < 0)
    {
      MLOG(ERROR, "meta data not available: " << strerror(-fd));
      return fd;
    }
    else
    {
      DMLOG(TRACE, "meta data opened: fd := " << fd);
    }

    uint64_t sz;
    int ec;

    ec = m_backend->seek (fd, 0, SEEK_END, &sz);
    m_backend->seek (fd, 0, SEEK_SET, 0);

    if (ec != 0)
    {
      MLOG(WARN, "could not determine size of meta data: " << strerror(-ec));
      m_backend->close(fd);
      return -EIO;
    }

    if (0 == sz)
    {
      MLOG(WARN, "ignoring empty meta data!");
      m_backend->close(fd);
      return -EIO;
    }

    DMLOG(TRACE, "transfering meta data with size " << sz);

    std::vector<char> buffer (sz, 0);

    PSProMessagePtr
      msg (create_new_pspro_message(server::command::MIGRATE_META_DATA, sz));
    size_t num_read;
    ec = m_backend->read (fd, msg->getCostumPtr(), sz, num_read);
    if (0 == ec)
    {
      ec = send_to_gui(msg);
      if (0 != ec)
      {
        MLOG(ERROR, "could not send meta data to GUI: " << strerror(-ec));
      }
    }
    else
    {
      MLOG(WARN, "reading meta data failed: " << strerror(-ec));
    }

    m_backend->close (fd);

    return ec;
  }

  int transfer_output_data_to_gui(transfer::request_ptr_t request)
  {
    if (request->state() == transfer::CANCELED)
    {
      return -ECANCELED;
    }

    int fd = m_backend->open("data.output");
    if (fd < 0)
    {
      MLOG(ERROR, "data not available: " << strerror(-fd));
      return fd;
    }
    else
    {
      MLOG(TRACE, "opened output data stream: fd := " << fd);
    }

    uint64_t total_size;
    int ec;

    ec = m_backend->seek (fd, 0, SEEK_END, &total_size);
    m_backend->seek (fd, 0, SEEK_SET, 0);

    size_t remaining = total_size;
    size_t offset = 0;
    while (remaining && request->state() != transfer::CANCELED)
    {
      size_t transfer_size = std::min (remaining, m_chunk_size);
      size_t message_size = sizeof(offset) + transfer_size;

      PSProMessagePtr
        msg (create_new_pspro_message(server::command::MIGRATE_DATA, message_size));
      memcpy(msg->getCostumPtr(), &offset, sizeof(offset));

      size_t num_read;
      ec = m_backend->read ( fd
                           , msg->getCostumPtr()+sizeof(offset)
                           , transfer_size
                           , num_read
                           );

      if (0 == ec)
      {
        ec = send_to_gui(msg);
        if (0 != ec)
        {
          MLOG(ERROR, "could not send data chunk to GUI: " << strerror(-ec));
          break;
        }

        offset += transfer_size;
        remaining -= transfer_size;
      }
      else
      {
        break;
      }
    }

    m_backend->close (fd);
    return ec;
  }

  int send_to_gui(PSProMessagePtr msg)
  {
    try
    {
      msg->sendMsg(m_server->communication(), m_send_timeout);
    }
    catch (std::exception const &ex)
    {
      return -EIO;
    }
    return 0;
  }

  bool m_stop_requested;
  int m_conn_timeout;
  int m_send_timeout;
  int m_recv_timeout;
  size_t m_chunk_size;

  PSProMigIF::StartServer* m_server;
  ufbmig::Backend *m_backend;

  transfer::request_queue_t m_transfer_requests;
  transfer::request_ptr_t m_current_transfer;

  boost::thread m_message_thread;
  boost::thread m_transfer_thread;

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
