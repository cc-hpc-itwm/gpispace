#include <plugin/plugin.hpp>
#include <gpi-space/pc/plugin/gpi.hpp>

#include <gpi-space/pc/client/api.hpp>

class GpiPluginImpl : FHG_PLUGIN
                    , public gpi::GPI
{
public:
  GpiPluginImpl (std::function<void()>, std::list<Plugin*>, std::map<std::string, std::string> config_variables)
    : api ("")
  {
    api.path (get<std::string> ("plugin.gpi.socket", config_variables).get());
    api.start ();
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
      return true;
  }

  virtual bool ping () override
  {
    return api.ping();
  }
private:
  gpi::pc::client::api_t api;
};

EXPORT_FHG_PLUGIN (gpi, GpiPluginImpl, "");
