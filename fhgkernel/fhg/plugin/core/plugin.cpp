#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include <iostream>
#include <stdexcept>
#include <algorithm>

#include <fhglog/LogMacros.hpp>
#include <fhg/assert.hpp>

#include <fhg/plugin/core/kernel.hpp>
#include <fhg/plugin/core/plugin.hpp>
#include <fhg/plugin/plugin.hpp>

#include <boost/bind.hpp>
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

    std::list<plugin::Plugin*> plugin_t::to_raw (std::list<boost::shared_ptr<plugin_t> > deps)
    {
      std::list<plugin::Plugin*> deps_raw;
      BOOST_FOREACH (boost::shared_ptr<plugin_t> p, deps)
      {
        deps_raw.push_back (p->m_plugin.get());
      }
      return deps_raw;
    }

    plugin_t::plugin_t ( void *my_handle
                       , kernel_t *kernel
                       , std::list<boost::shared_ptr<plugin_t> > deps
                       , std::map<std::string, std::string> config_variables
                       )
      : m_handle (my_handle)
      , m_plugin ( m_handle.sym
                   <plugin::Plugin* (boost::function<void()> request_stop, std::list<plugin::Plugin*>, std::map<std::string, std::string>)>
                   ("fhg_get_plugin_instance") (boost::bind (&kernel_t::stop, kernel), to_raw (deps), config_variables)
                 )
    {}
  }
}
