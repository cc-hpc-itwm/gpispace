#include "isim.hpp"
#include "ufbmig_msg_types.hpp"
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

#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/function.hpp>

#include "observable.hpp"
#include "observer.hpp"

typedef boost::recursive_mutex mutex_type;
typedef boost::unique_lock<mutex_type> lock_type;
typedef boost::condition_variable_any condition_type;

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
    typedef boost::function<void (void)> callback_fun_t;

    request_t ()
      : m_state(0)
    {}

    int state() const { lock_type lck(m_mtx); return m_state; }
    void set_state(int s) { lock_type lck(m_mtx); m_state = s; }

    void set_callback(callback_fun_t const & f)
    {
      lock_type lck(m_mtx); m_callback=f;
    }

    void completed() { m_callback(); }
  private:
    mutable mutex_type m_mtx;

    int m_state; // 0 pending 1 executing 2 canceled
    callback_fun_t m_callback;
  };

  typedef boost::shared_ptr<request_t> request_ptr_t;
  typedef fhg::thread::queue<request_ptr_t, std::list> request_queue_t;
}

class UfBMigFrontImpl : FHG_PLUGIN
                      , public ufbmig::Frontend
                      , public observe::Observer
{
public:
  UfBMigFrontImpl ()
  {
  }

  FHG_PLUGIN_START()
  {
    m_migrate_xml_buffer.clear();

    m_send_timeout = fhg_kernel()->get<int>("send_timeout", -1);
    m_recv_timeout = fhg_kernel()->get<int>("recv_timeout", -1);

    MLOG( TRACE
        ,  "timeouts:"
        << " send = " << m_send_timeout
        << " recv = " << m_recv_timeout
        );

    m_chunk_size = fhg_kernel()->get<std::size_t>("chunk_size", "4194304");
    DMLOG(TRACE, "using chunk size for GUI messages of: " << m_chunk_size);

    m_current_transfer = transfer::request_ptr_t(new transfer::request_t());
    m_current_transfer->set_state(transfer::CANCELED);

    m_backend = fhg_kernel()->acquire<ufbmig::Backend>("ufbmig_back");
    assert (m_backend);
    m_backend->registerFrontend(this);

    m_isim = fhg_kernel()->acquire<isim::ISIM>("isim");
    assert (m_isim);

    m_stop_requested = false;
    m_transfer_thread = boost::thread (&UfBMigFrontImpl::transfer_thread, this);
    m_message_thread = boost::thread (&UfBMigFrontImpl::message_thread, this);

    // send_waiting_for_initialize();
    fhg_kernel()->schedule ( "prepare_backend"
                           , boost::bind( &UfBMigFrontImpl::prepare_backend
                                        , this
                                        )
                           , 1
                           );

    // try to start to observe log events

    if (observe::Observable* o = fhg_kernel()->acquire<observe::Observable>("logd"))
    {
      MLOG(INFO, "ufbmig_front starts to observe: logd plugin");
      start_to_observe(o);
    }

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    stop_to_observe();

    m_stop_requested = true;

    m_transfer_thread.interrupt();
    m_transfer_thread.join ();

    m_message_thread.interrupt();
    m_message_thread.join ();

    FHG_PLUGIN_STOPPED();
  }

  FHG_ON_PLUGIN_LOADED(plugin)
  {
    if (plugin == "logd")
    {
      if (observe::Observable* o = fhg_kernel()->acquire<observe::Observable>("logd"))
      {
        MLOG(INFO, "ufbmig_front starts to observe: logd plugin");
        start_to_observe(o);
      }
    }
  }

  void notify(const observe::Observable *, boost::any const &evt)
  {
    try
    {
      fhg::log::LogEvent const & e (boost::any_cast<fhg::log::LogEvent>(evt));
      int ec = send_logoutput(e.message());
      if (ec < 0)
      {
        MLOG( WARN
            , "could not send log message '"
            << e.message()
            << "' to GUI: "
            << strerror(-ec)
            );
      }
    }
    catch (boost::bad_any_cast const &ex)
    {
    }
    catch (std::exception const & ex)
    {
      MLOG(ERROR, "could not send event: " << ex.what());
    }
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

    m_transfer_requests.clear();
    m_current_transfer->set_state(transfer::CANCELED);

    return m_backend->cancel();
  }

  int prepare_backend ()
  {
    assert (m_backend);
    return m_backend->prepare ();
  }

  void prepare_backend_done (int ec)
  {
    if (! ec)
    {
      LOG (TRACE, "backend prepared");
      send_waiting_for_initialize ();
    }
    else
    {
      LOG (ERROR, "backend preparation failed: " << ec);
      m_backend->stop();

      sleep (5);

      fhg_kernel()->terminate();
    }
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

      m_transfer_requests.clear ();
      m_current_transfer->set_state(transfer::CANCELED);
      transfer::request_ptr_t transfer(new transfer::request_t());
      transfer->set_callback (boost::bind( &UfBMigFrontImpl::send_migrate_success
                                         , this
                                         )
                             );
      m_transfer_requests.put(transfer);
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
  isim::msg_t *create_error_msg ( int cmd
                                , int ec
                                )
  {
    if (ec < 0)
      ec = -ec;
    std::string error (strerror(ec));

    return create_msg (cmd, error.c_str (), error.size () + 1);
  }

  isim::msg_t *create_msg ( int cmd
                          , const void *data
                          , size_t size
                          )
  {
    isim::msg_t *msg = isim::msg_new (cmd, size);
    memcpy (msg_data (msg), data, size);
    return msg;
  }

  int send_waiting_for_initialize()
  {
    m_isim->idle ();
    isim::msg_t *msg = isim::msg_new (server::command::WAITING_FOR_INITIALIZE);
    return m_isim->send (&msg, m_send_timeout);
  }

  int send_initializing()
  {
    m_isim->busy();
    isim::msg_t *msg = isim::msg_new (server::command::INITIALIZING);
    return m_isim->send (&msg, m_send_timeout);
  }

  int send_initialize_success()
  {
    m_isim->idle ();
    isim::msg_t *msg = isim::msg_new (server::command::INITIALIZE_SUCCESS);
    return m_isim->send (&msg, m_send_timeout);
  }

  int send_initialize_failure(int ec)
  {
    m_isim->idle();
    isim::msg_t *msg =
      create_error_msg (server::command::INITIALIZE_FAILURE, ec);
    return m_isim->send (&msg, m_send_timeout);
  }

  int send_processing_salt_mask()
  {
    m_isim->busy();
    isim::msg_t *msg = isim::msg_new (server::command::PROCESSING_SALT_MASK);
    return m_isim->send (&msg, m_send_timeout);
  }

  int send_process_salt_mask_success()
  {
    m_isim->idle();
    isim::msg_t *msg =
      isim::msg_new (server::command::PROCESS_SALT_MASK_SUCCESS);
    return m_isim->send (&msg, m_send_timeout);
  }

  int send_process_salt_mask_failure(int ec)
  {
    m_isim->idle();
    isim::msg_t *msg =
      create_error_msg (server::command::PROCESS_SALT_MASK_FAILURE, ec);
    return m_isim->send (&msg, m_send_timeout);
  }

  int send_migrating()
  {
    m_isim->busy();
    isim::msg_t *msg =
      isim::msg_new (server::command::MIGRATING);
    return m_isim->send (&msg, m_send_timeout);
  }

  int send_migrate_success()
  {
    m_isim->idle();
    isim::msg_t *msg =
      isim::msg_new (server::command::MIGRATE_SUCCESS);
    return m_isim->send (&msg, m_send_timeout);
  }

  int send_migrate_failure(int ec)
  {
    m_isim->idle();
    isim::msg_t *msg =
      create_error_msg (server::command::MIGRATE_FAILURE, ec);
    return m_isim->send (&msg, m_send_timeout);
  }

  int send_finalizing()
  {
    m_isim->busy();
    isim::msg_t *msg =
      isim::msg_new (server::command::FINALIZING);
    return m_isim->send (&msg, m_send_timeout);
  }

  int send_finalize_success()
  {
    m_isim->idle();
    isim::msg_t *msg =
      isim::msg_new (server::command::FINALIZE_SUCCESS);
    m_isim->send (&msg, m_send_timeout);
    return send_waiting_for_initialize ();
  }

  int send_finalize_failure(int ec)
  {
    m_isim->idle();
    isim::msg_t *msg =
      create_error_msg (server::command::FINALIZE_FAILURE, ec);
    m_isim->send (&msg, m_send_timeout);
    return send_waiting_for_initialize ();
  }

  int send_progress(int p)
  {
    isim::msg_t *msg = create_msg (server::command::PROGRESS, &p, sizeof(p));
    return m_isim->send (&msg, m_send_timeout);
  }

  int send_logoutput(std::string const &text)
  {
    isim::msg_t *msg = create_msg (server::command::LOGOUTPUT
                                  , text.c_str ()
                                  , text.size () + 1
                                  );
    return m_isim->send (&msg, m_send_timeout);
  }

  int send_abort_accepted()
  {
    isim::msg_t *msg =
      isim::msg_new (server::command::ABORT_ACCEPTED);
    return m_isim->send (&msg, m_send_timeout);
  }

  int send_abort_refused(int ec)
  {
    isim::msg_t *msg = create_error_msg (server::command::ABORT_REFUSED, ec);
    return m_isim->send (&msg, m_send_timeout);
  }

  void message_thread ()
  {
    MLOG(TRACE, "waiting for messages...");

    size_t fail_counter = 0;
    while (!m_stop_requested && fail_counter < 1)
    {
      std::string payload;

      isim::msg_t *msg = m_isim->recv (m_recv_timeout);
      if (! msg)
      {
        ++fail_counter;
        continue;
      }
      else
      {
        fail_counter = 0;
      }

      if (isim::msg_size (msg) > 0)
      {
        payload = std::string( (char*)isim::msg_data (msg)
                             , isim::msg_size (msg)
                             );
      }

      handle_ISIM_command(isim::msg_type (msg), payload);

      isim::msg_destroy (&msg);
    }

    if (fail_counter)
    {
      int ec = errno;
      MLOG ( ERROR
           , "terminating, since there were " << fail_counter << " failures"
           << " while receiving messages from the GUI: " << strerror(ec)
           );
      m_backend->stop();

      sleep (5);

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
      send_initializing();

      ec = initialize(payload);
      if (0 != ec)
      {
        send_initialize_failure(ec);
      }
      break;
    case client::command::MIGRATE:
      send_migrating();
      ec = calculate(payload);
      if (0 != ec)
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
      send_processing_salt_mask();
      ec = update_salt_mask(payload.c_str(), payload.size());
      if (0 != ec)
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
      send_finalizing();
      ec = finalize();
      if (0 != ec)
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
      m_current_transfer->completed();

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

    isim::msg_t *msg = isim::msg_new (server::command::MIGRATE_META_DATA, sz);
    size_t num_read;
    ec = m_backend->read (fd, (char*)isim::msg_data (msg), sz, num_read);
    if (0 == ec)
    {
      ec = m_isim->send (&msg, m_send_timeout);
      if (0 != ec)
      {
        MLOG(ERROR, "could not send meta data to GUI: " << strerror(-ec));
        isim::msg_destroy (&msg);
      }
    }
    else
    {
      MLOG(WARN, "reading meta data failed: " << strerror(-ec));
      isim::msg_destroy (&msg);
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

      isim::msg_t *msg = isim::msg_new ( server::command::MIGRATE_DATA
                                       , message_size
                                       );
      memcpy (isim::msg_data (msg), &offset, sizeof(offset));

      size_t num_read;
      ec = m_backend->read ( fd
                           , (char*)isim::msg_data (msg)+sizeof(offset)
                           , transfer_size
                           , num_read
                           );

      if (0 == ec)
      {
        ec = m_isim->send (&msg, m_send_timeout);
        if (0 != ec)
        {
          MLOG(ERROR, "could not send data chunk to GUI: " << strerror(-ec));
          isim::msg_destroy (&msg);
          break;
        }

        offset += transfer_size;
        remaining -= transfer_size;
      }
      else
      {
        isim::msg_destroy (&msg);
        break;
      }
    }

    if (request->state() == transfer::CANCELED)
    {
      MLOG(WARN, "data transfer to GUI has been cancelled");
    }

    m_backend->close (fd);
    return ec;
  }

  bool m_stop_requested;
  int m_send_timeout;
  int m_recv_timeout;
  size_t m_chunk_size;

  isim::ISIM      *m_isim;
  ufbmig::Backend *m_backend;

  transfer::request_queue_t m_transfer_requests;
  transfer::request_ptr_t m_current_transfer;

  boost::thread m_message_thread;
  boost::thread m_transfer_thread;

  std::string m_migrate_xml_buffer;
};

EXPORT_FHG_PLUGIN( ufbmig_front
                 , UfBMigFrontImpl
                 , ""
                 , "provides the frontend functionality for the UfBMig"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , "isim,ufbmig_back"
                 , ""
                 );
