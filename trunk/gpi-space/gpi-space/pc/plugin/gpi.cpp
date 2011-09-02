#include <fhglog/minimal.hpp>

#include <fhg/util/read_bool.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/capability.hpp>
#include "gpi.hpp"

#include <gpi-space/pc/client/api.hpp>

class GpiPluginImpl : FHG_PLUGIN
                    , public gpi::GPI
                    , public fhg::plugin::Capability
{
public:
  GpiPluginImpl()
    : api ("")
  {}

  virtual ~GpiPluginImpl()
  {
    api.stop();
  }

  const char * capability_name () const { return "GPI";  }
  const char * capability_type () const { return "PGAS"; }

  FHG_PLUGIN_START()
  {
    api.path (fhg_kernel()->get("socket", "/var/tmp/gpi-space/control"));
    if (fhg_kernel()->get("startmode", "nowait")  == "wait")
    {
      LOG(INFO, "gpi plugin starting in synchronous mode, this might take forever!");
      while (!try_start())
      {
        if (usleep (2 * 1000 * 1000) < 0)
        {
          FHG_PLUGIN_FAILED(EADDRNOTAVAIL);
        }
      }
    }
    else
    {
      if (! try_start())
      {
        fhg_kernel()->schedule( boost::bind ( &GpiPluginImpl::restart_loop
                                            , this
                                            )
                              , 0
                              );
        FHG_PLUGIN_INCOMPLETE();
      }
    }
    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    api.stop();
    FHG_PLUGIN_STOPPED();
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

private:
  bool try_start ()
  {
    try
    {
      api.start();
      return true;
    }
    catch (std::exception const &ex)
    {
      LOG( WARN
         , "could not start gpi connection on `" << api.path() << "': " << ex.what()
         );
      return false;
    }
  }

  void restart_loop ()
  {
    if (try_start())
    {
      fhg_kernel()->start_completed(0);
    }
    else
    {
      fhg_kernel()->schedule( boost::bind ( &GpiPluginImpl::restart_loop
                                          , this
                                          )
                            , 5
                            );
    }
  }

  gpi::pc::client::api_t api;
};

EXPORT_FHG_PLUGIN( gpi
                 , GpiPluginImpl
                 , "Plugin to access the gpi-space"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "v.0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
