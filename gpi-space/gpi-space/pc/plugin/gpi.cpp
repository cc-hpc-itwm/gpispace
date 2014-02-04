#include <fhglog/LogMacros.hpp>

#include <boost/thread.hpp>

#include <fhg/util/read_bool.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/capability.hpp>
#include "gpi.hpp"

#include <gpi-space/pc/client/api.hpp>

typedef boost::recursive_mutex         mutex_type;
typedef boost::unique_lock<mutex_type> lock_type;

class GpiPluginImpl : FHG_PLUGIN
                    , public gpi::GPI
                    , public fhg::plugin::Capability
{
public:
  GpiPluginImpl (Kernel *fhg_kernel, std::list<Plugin*>)
    : Capability ("GPI", "PGAS")
    , api ("")
    , _try_start_loop (NULL)
  {
    const std::string socket_path
      ( fhg_kernel->get ( "plugin.gpi.socket"
                        , "/var/tmp/S-gpi-space."
                        + boost::lexical_cast<std::string>(getuid())
                        + "."
                        + boost::lexical_cast<std::string>(0) // numa socket
                        )
      );
    const bool start_synchronous
      (fhg_kernel->get("plugin.gpi.startmode", "nowait") == "wait");
    boost::optional<std::size_t> max_retries_until_defer_startup
      ( start_synchronous
      ? boost::optional<std::size_t>()
      : fhg_kernel->get<std::size_t> ("plugin.gpi.retries_to_defer", "1")
      );

    api.path (socket_path);

    if (start_synchronous)
    {
      LOG(INFO, "gpi plugin starting in synchronous mode, this might take forever!");
      restart_loop();
    }
    else
    {
      std::size_t retries_until_defer_startup
        (std::max (max_retries_until_defer_startup.get_value_or (0), std::size_t (1)));

      bool connected = false;

      do
      {
        retries_until_defer_startup--;

        connected = try_start();

        if (! connected && retries_until_defer_startup)
        {
          boost::this_thread::sleep (boost::posix_time::seconds (5));
        }
        else
        {
          break;
        }
      } while (true);

      if (! connected)
      {
        _try_start_loop = new boost::thread (&GpiPluginImpl::restart_loop, this);
      }
    }
  }

  virtual ~GpiPluginImpl()
  {
    if (_try_start_loop)
    {
      _try_start_loop->interrupt();
      if (_try_start_loop->joinable())
      {
        _try_start_loop->join();
      }
      delete _try_start_loop;
      _try_start_loop = NULL;
    }
    api.stop();
  }

  gpi::pc::type::handle_id_t alloc ( const gpi::pc::type::segment_id_t seg_id
                                   , const gpi::pc::type::size_t sz
                                   , const std::string & desc
                                   , const gpi::pc::type::flags_t flgs
                                   )
  {
    return api.alloc(seg_id, sz, desc, flgs);
  }

  void free (const gpi::pc::type::handle_id_t hdl)
  {
    return api.free(hdl);
  }

  gpi::pc::type::handle::list_t list_allocations (const gpi::pc::type::segment_id_t seg)
  {
    return api.list_allocations(seg);
  }

  gpi::pc::type::queue_id_t memcpy ( gpi::pc::type::memory_location_t const & dst
                                   , gpi::pc::type::memory_location_t const & src
                                   , const gpi::pc::type::size_t amount
                                   , const gpi::pc::type::queue_id_t queue
                                   )
  {
    return api.memcpy(dst, src, amount, queue);
  }

  gpi::pc::type::handle_t memset (const gpi::pc::type::handle_t h
                                 , int value
                                 , size_t count
                                 )
  {
    return api.memset(h,value,count);
  }

  void * ptr(const gpi::pc::type::handle_t h)
  {
    return api.ptr(h);
  }

  gpi::pc::type::size_t wait (const gpi::pc::type::queue_id_t q)
  {
    return api.wait(q);
  }

  std::vector<gpi::pc::type::size_t> wait ()
  {
    return api.wait();
  }

  gpi::pc::type::segment_id_t register_segment( std::string const & name
                                              , const gpi::pc::type::size_t sz
                                              , const gpi::pc::type::flags_t flgs
                                              )
  {
    return api.register_segment(name,sz,flgs);
  }

  void unregister_segment(const gpi::pc::type::segment_id_t id)
  {
    return api.unregister_segment(id);
  }

  void attach_segment(const gpi::pc::type::segment_id_t id)
  {
    return api.attach_segment(id);
  }

  void detach_segment(const gpi::pc::type::segment_id_t id)
  {
    return api.detach_segment(id);
  }

  gpi::pc::type::segment::list_t list_segments ()
  {
    return api.list_segments();
  }

  gpi::pc::type::info::descriptor_t collect_info ()
  {
    return api.collect_info();
  }

  void garbage_collect ()
  {
    return api.garbage_collect();
  }

  bool is_connected () const
  {
    return api.is_connected();
  }

  bool connect ()
  {
    lock_type lock (m_state_mtx);

    if (! is_connected())
    {
      return try_start ();
    }
    else
    {
      return true;
    }
  }

  bool ping ()
  {
    return api.ping();
  }
private:
  bool try_start ()
  {
    lock_type lock (m_state_mtx);

    try
    {
      api.start ();
      return true;
    }
    catch (std::exception const &ex)
    {
      MLOG ( WARN
           , "could not start gpi connection on `" << api.path() << "': " << ex.what()
           );
      return false;
    }
  }

  void restart_loop ()
  {
    while (! try_start())
    {
      boost::this_thread::sleep (boost::posix_time::milliseconds (2500));
    }
  }

  mutable mutex_type     m_state_mtx;
  gpi::pc::client::api_t api;

  boost::thread* _try_start_loop;
};

EXPORT_FHG_PLUGIN (gpi, GpiPluginImpl, "");
