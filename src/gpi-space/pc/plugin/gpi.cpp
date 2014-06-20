#include <fhglog/LogMacros.hpp>

#include <boost/thread.hpp>

#include <fhg/util/read_bool.hpp>
#include <plugin/plugin.hpp>
#include <gpi-space/pc/plugin/gpi.hpp>

#include <gpi-space/pc/client/api.hpp>

#include <boost/thread/scoped_thread.hpp>

typedef boost::recursive_mutex         mutex_type;
typedef boost::unique_lock<mutex_type> lock_type;

class GpiPluginImpl : FHG_PLUGIN
                    , public gpi::GPI
{
public:
  GpiPluginImpl (std::function<void()>, std::list<Plugin*>, std::map<std::string, std::string> config_variables)
    : api ("")
    , _try_start_loop (nullptr)
  {
    const std::string socket_path
      ( get<std::string> ("plugin.gpi.socket", config_variables)
      .get_value_or ( "/var/tmp/S-gpi-space."
                    + boost::lexical_cast<std::string>(getuid())
                    + "."
                    + boost::lexical_cast<std::string>(0) // numa socket
                    )
      );
    const bool start_synchronous
      (get<std::string> ("plugin.gpi.startmode", config_variables) == std::string ("wait"));
    boost::optional<std::size_t> max_retries_until_defer_startup
      ( start_synchronous
      ? boost::optional<std::size_t>()
      : get<std::size_t> ("plugin.gpi.retries_to_defer", config_variables).get_value_or (1)
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
        _try_start_loop = new boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
          (&GpiPluginImpl::restart_loop, this);
      }
    }
  }

  virtual ~GpiPluginImpl()
  {
    delete _try_start_loop;
    _try_start_loop = nullptr;
  }

  virtual gpi::pc::type::handle_id_t alloc ( const gpi::pc::type::segment_id_t seg_id
                                   , const gpi::pc::type::size_t sz
                                   , const std::string & desc
                                   , const gpi::pc::type::flags_t flgs
                                   ) override
  {
    return api.alloc(seg_id, sz, desc, flgs);
  }

  virtual void free (const gpi::pc::type::handle_id_t hdl) override
  {
    return api.free(hdl);
  }

  virtual gpi::pc::type::handle::list_t list_allocations (const gpi::pc::type::segment_id_t seg) override
  {
    return api.list_allocations(seg);
  }

  virtual gpi::pc::type::queue_id_t memcpy ( gpi::pc::type::memory_location_t const & dst
                                   , gpi::pc::type::memory_location_t const & src
                                   , const gpi::pc::type::size_t amount
                                   , const gpi::pc::type::queue_id_t queue
                                   ) override
  {
    return api.memcpy(dst, src, amount, queue);
  }

  virtual gpi::pc::type::handle_t memset (const gpi::pc::type::handle_t h
                                 , int value
                                 , size_t count
                                 ) override
  {
    return api.memset(h,value,count);
  }

  virtual void * ptr(const gpi::pc::type::handle_t h) override
  {
    return api.ptr(h);
  }

  virtual gpi::pc::type::size_t wait (const gpi::pc::type::queue_id_t q) override
  {
    return api.wait(q);
  }

  virtual std::vector<gpi::pc::type::size_t> wait () override
  {
    return api.wait();
  }

  virtual gpi::pc::type::segment_id_t register_segment( std::string const & name
                                              , const gpi::pc::type::size_t sz
                                              , const gpi::pc::type::flags_t flgs
                                              ) override
  {
    return api.register_segment(name,sz,flgs);
  }

  virtual void unregister_segment(const gpi::pc::type::segment_id_t id) override
  {
    return api.unregister_segment(id);
  }

  virtual void attach_segment(const gpi::pc::type::segment_id_t id) override
  {
    return api.attach_segment(id);
  }

  virtual void detach_segment(const gpi::pc::type::segment_id_t id) override
  {
    return api.detach_segment(id);
  }

  virtual gpi::pc::type::segment::list_t list_segments () override
  {
    return api.list_segments();
  }

  virtual gpi::pc::type::info::descriptor_t collect_info () override
  {
    return api.collect_info();
  }

  virtual void garbage_collect () override
  {
    return api.garbage_collect();
  }

  virtual bool is_connected () const override
  {
    return api.is_connected();
  }

  virtual bool connect () override
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

  virtual bool ping () override
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

  virtual void restart_loop ()
  {
    while (! try_start())
    {
      boost::this_thread::sleep (boost::posix_time::milliseconds (2500));
    }
  }

  mutable mutex_type     m_state_mtx;
  gpi::pc::client::api_t api;

  boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>*
    _try_start_loop;
};

EXPORT_FHG_PLUGIN (gpi, GpiPluginImpl, "");
