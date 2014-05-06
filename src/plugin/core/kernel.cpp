#include <unistd.h> // getuid, alarm, setsid, fork
#include <sys/types.h> // uid_t
#include <sys/time.h>
#include <dlfcn.h>
#include <cstring>
#include <algorithm>

#include <fhglog/LogMacros.hpp>

#include <plugin/plugin.hpp>
#include <plugin/core/plugin.hpp>
#include <plugin/core/kernel.hpp>

#include <fhg/util/daemonize.hpp>
#include <fhg/util/split.hpp>

#include <boost/range/adaptor/reversed.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

#define START_SUCCESSFUL 0

namespace fhg
{
  namespace core
  {
    void wait_until_stopped::wait()
    {
      _stop_requested.wait();
      _stopped.notify();
    }

    void wait_until_stopped::stop()
    {
      _stop_requested.notify();
      _stopped.wait();
    }

    std::function<void()> wait_until_stopped::make_request_stop()
    {
      return [this] { stop(); };
    }

    kernel_t::kernel_t ( fhg::core::kernel_t::search_path_t search_path
                       , std::function<void()> request_stop
                       , std::map<std::string, std::string> config_variables
                       )
      : _stop (request_stop)
      , m_config (config_variables)
      , m_search_path (search_path)
    {
    }

    kernel_t::~kernel_t ()
    {
      for ( std::string plugin_to_unload
          : m_load_order | boost::adaptors::reversed
          )
      {
        m_plugins.erase (m_plugins.find (plugin_to_unload));
      }
    }

    void kernel_t::load_plugin_by_name (std::string const & name)
    try
    {
      if (m_search_path.empty())
      {
        throw std::runtime_error ("search path is empty");
      }

      for (std::string const &dir : m_search_path)
      {
        fs::path plugin_path (dir);
        plugin_path /= name + ".so";

        if (fs::is_regular_file (plugin_path))
        {
          load_plugin_from_file (plugin_path.string ());
          return;
        }
      }
    }
    catch (std::runtime_error const& ex)
    {
      throw std::runtime_error
        ("plugin " + name + " failed to start: " + ex.what());
    }

    void kernel_t::load_plugin_from_file (std::string const &file)
    try
    {
      bool load_lazy (get <bool> ("kernel.load.lazy", m_config).get_value_or (true));

      std::string full_path_to_file = fs::absolute (fs::path (file)).string ();

      if (not fs::is_regular_file (full_path_to_file))
      {
        throw std::runtime_error (full_path_to_file + " is not a regular file");
      }


      // dlopen file
      void *handle (dlopen(full_path_to_file.c_str(), RTLD_GLOBAL | (load_lazy?RTLD_LAZY:RTLD_NOW)));
      if (!handle)
      {
        std::string msg ("dlopen () failed: ");
        msg += dlerror ();
        throw std::runtime_error(msg);
      }

      dlerror();

      union
      {
        void* _ptr;
        const fhg_plugin_descriptor_t* (*_fun)();
      } query_plugin;

      query_plugin._ptr = dlsym(handle, "fhg_query_plugin_descriptor");

      if (char* error = dlerror())
      {
        dlclose(handle);
        throw std::runtime_error("could not get query function: " + std::string(error));
      }

      const fhg_plugin_descriptor_t *desc = query_plugin._fun();
      const std::string plugin_name (desc->name);

      if (m_plugins.find (plugin_name) != m_plugins.end())
      {
        throw std::runtime_error
          ("another plugin with the same name is already loaded: " + plugin_name);
      }

      std::list<boost::shared_ptr<plugin_t>> dependencies;

      std::list<std::string> const depends
        (fhg::util::split<std::string, std::string> (desc->depends, ','));

      for (std::string const &dep : depends)
      {
        if (m_plugins.find (dep) == m_plugins.end())
        {
          load_plugin_by_name (dep);
        }

        dependencies.push_back (m_plugins.find (dep)->second);
      }

      boost::shared_ptr<plugin_t> p (new plugin_t( handle
                                     , this
                                     , dependencies
                                     , m_config
                                          )
                        );

      m_plugins.emplace (plugin_name, p);
      m_load_order.push_back (plugin_name);

      MLOG( TRACE
          , "loaded plugin '" << plugin_name << "'"
          << " (from: '" << full_path_to_file << "')"
          );
    }
    catch (std::runtime_error const& ex)
    {
      throw std::runtime_error
        ("plugin " + file + " failed to start: " + ex.what());
    }
  }
}
