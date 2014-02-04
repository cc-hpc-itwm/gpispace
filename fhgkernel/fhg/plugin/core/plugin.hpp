#ifndef FHG_PLUGIN_CORE_PLUGIN_HPP
#define FHG_PLUGIN_CORE_PLUGIN_HPP 1

#include <list>
#include <string>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

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

      plugin_t ( void * handle
               , fhg::plugin::Kernel *kernel
               , std::list<plugin_t::ptr_t> deps
               );

      void handle_plugin_loaded (plugin_t::ptr_t);
      void handle_plugin_preunload (plugin_t::ptr_t);
    private:
      void check_dependencies();

      struct close_on_dtor_dlhandle
      {
        close_on_dtor_dlhandle (void*);
        ~close_on_dtor_dlhandle();
        void* _;

        template<typename T> T* sym (std::string);
      } m_handle;

      boost::scoped_ptr<fhg::plugin::Plugin> m_plugin;

      static std::list<plugin::Plugin*> to_raw (std::list<plugin_t::ptr_t>);
    };
  }
}

#endif
