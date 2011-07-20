#include <fhglog/minimal.hpp>

#include <fhg/plugin/plugin.hpp>
#include "gpi.hpp"

#include <gpi-space/pc/client/api.hpp>

class GpiPluginImpl : FHG_PLUGIN
                    , public gpi::GPI
{
public:
  GpiPluginImpl()
    : api ("")
  {}

  FHG_PLUGIN_START(kernel)
  {
    try_start ();
    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP(kernel)
  {
    FHG_PLUGIN_STOPPED();
  }

  gpi::pc::type::handle_id_t alloc ( const gpi::pc::type::segment_id_t
                                   , const gpi::pc::type::size_t
                                   , const std::string & desc
                                   , const gpi::pc::type::flags_t
                                   )
  {
    return 0;
  }

  void free (const gpi::pc::type::handle_id_t)
  {}

  gpi::pc::type::handle::list_t list_allocations (const gpi::pc::type::segment_id_t seg)
  {
    return gpi::pc::type::handle::list_t();
  }

  gpi::pc::type::queue_id_t memcpy ( gpi::pc::type::memory_location_t const & dst
                                   , gpi::pc::type::memory_location_t const & src
                                   , const gpi::pc::type::size_t amount
                                   , const gpi::pc::type::queue_id_t queue
                                   )
  {
    return 0;
  }

  gpi::pc::type::handle_t memset (const gpi::pc::type::handle_t h
                                 , int value
                                 , size_t count
                                 )
  {
    return h;
  }

  void * ptr(const gpi::pc::type::handle_t h)
  {
    return (void*)(0);
  }

  gpi::pc::type::size_t wait (const gpi::pc::type::queue_id_t)
  {
    return 0;
  }

  std::vector<gpi::pc::type::size_t> wait ()
  {
    return std::vector<gpi::pc::type::size_t>();
  }

  gpi::pc::type::segment_id_t register_segment( std::string const & name
                                              , const gpi::pc::type::size_t sz
                                              , const gpi::pc::type::flags_t
                                              )
  {
    return 0;
  }

  void unregister_segment(const gpi::pc::type::segment_id_t)
  {}

  void attach_segment(const gpi::pc::type::segment_id_t id){ }
  void detach_segment(const gpi::pc::type::segment_id_t id){ }
  gpi::pc::type::segment::list_t list_segments ()
  {
    return gpi::pc::type::segment::list_t();
  }

  gpi::pc::type::info::descriptor_t collect_info ()
  {
    return gpi::pc::type::info::descriptor_t();
  }

private:
  void try_start ()
  {
    try
    {
      api.start();
    }
    catch (std::exception const &ex)
    {
      LOG(WARN, "could not start gpi connection: " << ex.what());

      fhg_kernel()->schedule( boost::bind ( &GpiPluginImpl::try_start
                                          , this
                                          )
                            , 5
                            );
    }
  }

  FHG_KERNEL_PTR m_kernel;
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
