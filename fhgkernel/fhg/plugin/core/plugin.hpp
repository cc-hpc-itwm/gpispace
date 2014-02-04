#ifndef FHG_PLUGIN_CORE_PLUGIN_HPP
#define FHG_PLUGIN_CORE_PLUGIN_HPP 1

#include <list>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <fhg/plugin/descriptor.hpp>
#include <fhg/plugin/kernel.hpp>

namespace fhg
{
  namespace plugin
  {
    class Plugin;
  }

  namespace core
  {
    class plugin_t
    {
    public:
      typedef boost::shared_ptr<plugin_t> ptr_t;

      plugin_t ( std::string const & name
               , std::string const & filename
               , const fhg_plugin_descriptor_t *
               , void * handle
               , fhg::plugin::Kernel *kernel
               , std::list<plugin_t::ptr_t> deps
               );
      ~plugin_t ();

      void handle_plugin_loaded (plugin_t::ptr_t);
      void handle_plugin_preunload (plugin_t::ptr_t);
    private:
      void check_dependencies();

      std::string m_name;
      std::string m_file_name;
      fhg::plugin::Plugin *m_plugin;
      const fhg_plugin_descriptor_t *m_descriptor;

      struct close_on_dtor_dlhandle
      {
        close_on_dtor_dlhandle (void*);
        ~close_on_dtor_dlhandle();
        void* _;
      } m_handle;

      std::list<ptr_t> m_dependencies;
    };
  }
}

#endif
