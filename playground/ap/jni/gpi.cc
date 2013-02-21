#include <malloc.h>
#include <errno.h>

#include <fstream>

#include <boost/thread.hpp>
#include <fhglog/fhglog.hpp>

#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/core/kernel.hpp>

#include <fhg/plugins/isim.hpp>
#include <fhg/plugins/reactor.hpp>
#include <fhg/plugins/ufbmig_msg_types.hpp>

#include <fhg/plugins/gpi.hpp>
#include <fhg/plugins/ufbmig_back.hpp>

typedef boost::recursive_mutex mutex_type;
typedef boost::unique_lock<mutex_type> lock_type;
typedef boost::condition_variable_any condition_type;

struct gpi_conn_t
{
  gpi_conn_t (int com_size)
    : stream (0)
    , isim (0)
    , krnl (0)
    , krnl_thrd (0)
    , com_sz (com_size)
    , com_seg (0)
    , com_seg_hdl (0)
    , com_gpi_hdl (0)
    , state (0)
  {}

  ufbmig::Backend *stream;
  isim::ISIM *isim;
  fhg::core::kernel_t *krnl;
  boost::thread *krnl_thrd;
  size_t com_sz;
  long     com_seg;
  uint64_t com_gpi_hdl;
  uint64_t com_seg_hdl;
  void *com_ptr;

  char xml_path [4096];

  int state;
  mutable mutex_type mutex;
  mutable condition_type done;
};

static gpi_conn_t *s_gpi = 0;

static isim::msg_t *s_react_on_message (isim::msg_t *msg)
{
  using namespace server::command;

  isim::msg_t *reply = 0;

  switch (s_gpi->isim->msg_type (msg))
  {
  case WAITING_FOR_INITIALIZE:
    {
      MLOG (INFO, "ufbmig waiting for initialize");

      std::ifstream ifs (s_gpi->xml_path);
      std::stringstream sstr;
      ifs >> std::noskipws >> sstr.rdbuf();

      std::string xml = sstr.str ();

      // open xml_path
      // read string buffer
      reply = s_gpi->isim->msg_new (client::command::INITIALIZE, xml.size ());
      memcpy (s_gpi->isim->msg_data (reply), xml.c_str (), xml.size ());

      MLOG (INFO, "sending " << xml);
    }
    break;
  case INITIALIZING:
    {
      MLOG (INFO, "ufbmig initializing...");
    }
    break;
  case INITIALIZE_SUCCESS:
    {
      MLOG (INFO, "ufbmig initialized!");
      reply = s_gpi->isim->msg_new (client::command::MIGRATE, 0);
    }
    break;
  case INITIALIZE_FAILURE:
    {
      MLOG ( WARN, "ufbmig initialization failed: "
           << (char*)(s_gpi->isim->msg_data (msg))
           );
    }
    break;
  case MIGRATING:
    MLOG (INFO, "ufbmig migrating...");
    break;
  case MIGRATE_SUCCESS:
    MLOG (INFO, "ufbmig migrated!");
    {
      //      reply = s_gpi->isim->msg_new (client::command::FINALIZE, 0);
      lock_type lck (s_gpi->mutex);
      s_gpi->state = 1;
      s_gpi->done.notify_all ();
    }
    break;
  case MIGRATE_FAILURE:
    {
      MLOG  (WARN, "ufbmig migration failed: "
            << (char*)(s_gpi->isim->msg_data (msg))
            );
      lock_type lck (s_gpi->mutex);
      s_gpi->state = 2;
      s_gpi->done.notify_all ();
    }
    break;
  case FINALIZING:
    MLOG (INFO, "ufbmig finalizing...");
    break;
  case FINALIZE_SUCCESS:
    MLOG (INFO, "ufbmig finalized!");
    break;
  case FINALIZE_FAILURE:
    MLOG ( WARN, "ufbmig finalization failed: "
         << (char*)(s_gpi->isim->msg_data (msg))
         );
    break;
  case PROCESSING_SALT_MASK:
    MLOG (INFO, "ufbmig processing salt mask...");
    break;
  case PROCESS_SALT_MASK_SUCCESS:
    MLOG (INFO, "ufbmig salt mask processed!");
    break;
  case PROCESS_SALT_MASK_FAILURE:
    MLOG ( WARN
         , "ufbmig salt mask failed: "
         << (char*)(s_gpi->isim->msg_data (msg))
         );
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
      memcpy (&value, s_gpi->isim->msg_data (msg), sizeof (value));
      MLOG (INFO, "ufbmig progress: " << value);
      break;
    }
  case LOGOUTPUT:
    MLOG (INFO, "ufbmig log message: " << (char*)(s_gpi->isim->msg_data (msg)));
    break;
  }

  return reply;
}

#ifdef __cplusplus
extern "C" {
#endif

  int        gpi_init ( int is_master
                      , const char *socket
                      , const char *plugin_dir
                      , const char *prep_wf
                      , const char *init_wf
                      , const char *calc_wf
                      , const char *mask_wf
                      , const char *done_wf
                      , const char *path_to_input
                      )
  {
    assert (s_gpi == 0);

    FHGLOG_SETUP ();

    s_gpi = new gpi_conn_t (4 * (1<<20));
    s_gpi->krnl = new fhg::core::kernel_t ("");
    s_gpi->krnl->add_search_path (plugin_dir);

    snprintf (s_gpi->xml_path, sizeof(s_gpi->xml_path), "%s", path_to_input);

    s_gpi->krnl->put ( "plugin.ufbmig_back.wf_prepare", prep_wf);
    s_gpi->krnl->put ( "plugin.ufbmig_back.wf_init", init_wf);
    s_gpi->krnl->put ( "plugin.ufbmig_back.wf_mask", mask_wf);
    s_gpi->krnl->put ( "plugin.ufbmig_back.wf_calc", calc_wf);
    s_gpi->krnl->put ( "plugin.ufbmig_back.wf_done", done_wf);

    s_gpi->krnl->put ("plugin.ufbmig_front.prep_timeout", "0");
    s_gpi->krnl->put ("plugin.gpi.startmode", "nowait");
    s_gpi->krnl->put ("plugin.gpi.socket", socket);

    if (is_master)
    {
      s_gpi->krnl->load_plugin ("kvsd");
      s_gpi->krnl->load_plugin ("gpi");
      s_gpi->krnl->load_plugin ("isim_dummy");
      s_gpi->krnl->load_plugin ("ufbmig_front");
      s_gpi->krnl->load_plugin ("ufbmig_back");

      s_gpi->isim = dynamic_cast<isim::ISIM*>
        (s_gpi->krnl->lookup_plugin ("isim")->get_plugin ());
      isim::Reactor *r = dynamic_cast<isim::Reactor*>
        (s_gpi->krnl->lookup_plugin ("isim")->get_plugin ());
//      r->set_reactor (s_react_on_message);
    }
    else
    {
      s_gpi->krnl->load_plugin ("gpi");
      s_gpi->krnl->load_plugin ("ufbmig_back");
    }

    s_gpi->krnl_thrd = new boost::thread
      (&fhg::core::kernel_t::run, s_gpi->krnl);
    s_gpi->stream = dynamic_cast<ufbmig::Backend*>
      (s_gpi->krnl->lookup_plugin ("ufbmig_back")->get_plugin ());

    return 0;
  }

  int gpi_wait (int rank)
  {
    lock_type lck (s_gpi->mutex);

    while (s_gpi->state == 0)
    {
      s_gpi->done.wait (lck);
    }
  }

  int gpi_open (const char *p)
  {
    return s_gpi->stream->open (p);
  }

  int gpi_close (int fd)
  {
    return s_gpi->stream->close (fd);
  }

  long gpi_seek (const int fd, const size_t off, const int whence)
  {
    size_t o;
    int rc = s_gpi->stream->seek (fd, off, whence, &o);
    if (rc == 0)
      return o;
    else
      throw std::runtime_error ("could not seek");
  }

  int gpi_read (int fd, void *buffer, size_t len)
  {
    size_t num;
    int rc = s_gpi->stream->read (fd, (char*)buffer, len, num);
    if (rc == 0)
      return num;
    else
      throw std::runtime_error ("could not read");
  }

  int gpi_write (int fd, const void *buffer, size_t len)
  {
    size_t num;
    int rc = s_gpi->stream->write (fd, (char*)buffer, len, num);
    if (rc == 0)
      return num;
    else
      throw std::runtime_error ("could not write");
  }

#ifdef __cplusplus
}
#endif
