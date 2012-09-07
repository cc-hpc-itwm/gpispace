#include "isim.hpp"
#include "ufbmig_msg_types.hpp"

#include <list>

#include <boost/thread.hpp>

#include <fhg/assert.hpp>
#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

namespace isim
{
  struct _msg_t
  {
    int    type;
    size_t size;
    void  *data;
  };
}

using namespace isim;

typedef boost::recursive_mutex mutex_type;
typedef boost::unique_lock<mutex_type> lock_type;
typedef boost::condition_variable_any condition_type;
typedef std::list <msg_t *> msg_queue_t;

class ISIM_Dummy : FHG_PLUGIN
                 , public isim::ISIM
{
public:
  ISIM_Dummy () {}
  ~ISIM_Dummy () {}

  FHG_PLUGIN_START()
  {
    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    FHG_PLUGIN_STOPPED();
  }

  FHG_ON_PLUGIN_LOADED(plugin) {}

  msg_t *recv (int timeout)
  {
    lock_type m_msg_queue_lock (m_mtx_msg_queue);
    while (m_msg_queue.empty ())
    {
      if (timeout >= 0)
      {
        boost::system_time const wait_until =
          boost::get_system_time()
          + boost::posix_time::milliseconds (timeout);
        if (m_msg_available.timed_wait (m_msg_queue_lock, wait_until))
          break;
        else
          return 0;
      }
      else
      {
        // block forever
        m_msg_available.wait (m_msg_queue_lock);
      }
    }

    if (not m_msg_queue.empty ())
    {
      msg_t *m = m_msg_queue.front (); m_msg_queue.pop_front ();
      return m;
    }
    else
    {
      return 0;
    }
  }

  int send (msg_t **p_msg, int timeout)
  {
    assert (p_msg);
    msg_t *msg = *p_msg;

    MLOG (TRACE, "sending message: " << msg_type (msg));

    react_on_message (msg);

    msg_destroy (p_msg);

    return 0;
  }

  void stop () {}
  void idle () {}
  void busy () {}

  msg_t *msg_new (int type, size_t size)
  {
    msg_t *msg = (msg_t*) (malloc (sizeof (msg_t)));
    assert (msg);

    if (size)
    {
      msg->data  = malloc (size);
      assert (msg->data);
    }
    else
    {
      msg->data = 0;
    }

    msg->type = type;
    msg->size = size;

    return msg;
  }

  void msg_destroy (msg_t **p_msg)
  {
    assert (p_msg);
    if (*p_msg)
    {
      msg_t *msg = *p_msg;
      if (msg->data)
        free (msg->data);
      free (msg);
      *p_msg = 0;
    }
  }

  int msg_type (msg_t *msg)
  {
    assert (msg);
    return msg->type;
  }

  void *msg_data (msg_t *msg)
  {
    assert (msg);
    return msg->data;
  }

  size_t msg_size (msg_t *msg)
  {
    assert (msg);
    return msg->size;
  }
private:
  void reply_with (msg_t *msg)
  {
    lock_type m_msg_queue_lock (m_mtx_msg_queue);
    m_msg_queue.push_back (msg);
    m_msg_available.notify_one ();
  }

  void react_on_message (msg_t *msg)
  {
    using namespace server::command;

    switch (msg_type (msg))
    {
    case WAITING_FOR_INITIALIZE:
      MLOG (INFO, "ufbmig waiting for initialize");
      {
        msg_t *reply = msg_new (client::command::INITIALIZE, 6);
        memcpy (msg_data (reply), "hello", 6);

        reply_with (reply);
      }
      break;
    case INITIALIZING:
      MLOG (INFO, "ufbmig initializing...");
      break;
    case INITIALIZE_SUCCESS:
      MLOG (INFO, "ufbmig initialized!");
      {
        msg_t *reply = msg_new (client::command::MIGRATE, 0);
        reply_with (reply);
      }
      break;
    case INITIALIZE_FAILURE:
      MLOG (WARN, "ufbmig initialization failed: " << (char*)(msg_data (msg)));
      break;
    case MIGRATING:
      MLOG (INFO, "ufbmig migrating...");
      break;
    case MIGRATE_SUCCESS:
      MLOG (INFO, "ufbmig migrated!");
      {
        msg_t *reply = msg_new (client::command::FINALIZE, 0);
        reply_with (reply);
      }
      break;
    case MIGRATE_FAILURE:
      MLOG (WARN, "ufbmig migration failed: " << (char*)(msg_data (msg)));
      break;
    case FINALIZING:
      MLOG (INFO, "ufbmig finalizing...");
      break;
    case FINALIZE_SUCCESS:
      MLOG (INFO, "ufbmig finalized!");
      break;
    case FINALIZE_FAILURE:
      MLOG (WARN, "ufbmig finalization failed: " << (char*)(msg_data (msg)));
      break;
    case PROCESSING_SALT_MASK:
      MLOG (INFO, "ufbmig processing salt mask...");
      break;
    case PROCESS_SALT_MASK_SUCCESS:
      MLOG (INFO, "ufbmig salt mask processed!");
      break;
    case PROCESS_SALT_MASK_FAILURE:
      MLOG (WARN, "ufbmig salt mask failed: " << (char*)(msg_data (msg)));
      break;
    case ABORT_ACCEPTED:
      MLOG (INFO, "ufbmig abort accepted");
      break;
    case ABORT_REFUSED:
      MLOG (INFO, "ufbmig abort refused!");
      break;
    case MIGRATE_META_DATA:
      MLOG (INFO, "ufbmig meta data!");
      break;
    case MIGRATE_DATA:
      MLOG (INFO, "ufbmig data chunk!");
      break;
    case PROGRESS:
      {
        int value = 0;
        memcpy (&value, msg_data (msg), sizeof (value));
        MLOG (INFO, "ufbmig progress: " << value);
        break;
      }
    case LOGOUTPUT:
      MLOG (INFO, "ufbmig log message: " << (char*)(msg_data (msg)));
      break;
    }
  }

  mutable mutex_type m_mtx_msg_queue;
  mutable condition_type m_msg_available;
  msg_queue_t m_msg_queue;
};

EXPORT_FHG_PLUGIN( isim
                 , ISIM_Dummy
                 , ""
                 , "provides dummy network functionality with the ISIM GUI"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
