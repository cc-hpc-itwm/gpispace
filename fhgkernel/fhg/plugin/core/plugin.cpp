#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include <iostream>
#include <stdexcept>
#include <algorithm>

#include <fhglog/LogMacros.hpp>
#include <fhg/assert.hpp>

#include <fhg/plugin/core/exception.hpp>
#include <fhg/plugin/core/plugin.hpp>
#include <fhg/plugin/plugin.hpp>

#include <boost/foreach.hpp>

namespace fhg
{
  namespace core
  {
    plugin_t::close_on_dtor_dlhandle::close_on_dtor_dlhandle (void* handle)
      : _ (handle)
    {}
    plugin_t::close_on_dtor_dlhandle::~close_on_dtor_dlhandle()
    {
      dlclose (_);
    }
    template<typename T>
      T* plugin_t::close_on_dtor_dlhandle::sym (std::string name)
    {
      dlerror();

      union
      {
        void* _ptr;
        T* _data;
      } symbol;

      symbol._ptr = dlsym (_, name.c_str());

      if (char* error = dlerror())
      {
        throw std::runtime_error ("could not get symbol '" + name + "': " + error);
      }

      return symbol._data;
    }

    std::list<plugin::Plugin*> plugin_t::to_raw (std::list<plugin_t::ptr_t> deps)
    {
      std::list<plugin::Plugin*> deps_raw;
      BOOST_FOREACH (ptr_t p, deps)
      {
        deps_raw.push_back (p->m_plugin.get());
      }
      return deps_raw;
    }

    plugin_t::plugin_t ( void *my_handle
                       , fhg::plugin::Kernel *kernel
                       , std::list<plugin_t::ptr_t> deps
                       )
      : m_handle (my_handle)
      , m_dependencies (deps)
      , m_plugin ( m_handle.sym
                   <plugin::Plugin* (plugin::Kernel*, std::list<plugin::Plugin*>)>
                     ("fhg_get_plugin_instance") (kernel, to_raw (deps))
                 )
    {}

    void plugin_t::handle_plugin_loaded (plugin_t::ptr_t other)
    {
      m_plugin->fhg_on_plugin_loaded (other->m_plugin.get());
    }

    void plugin_t::handle_plugin_preunload (plugin_t::ptr_t other)
    {
      m_plugin->fhg_on_plugin_preunload (other->m_plugin.get());
    }
  }
}
