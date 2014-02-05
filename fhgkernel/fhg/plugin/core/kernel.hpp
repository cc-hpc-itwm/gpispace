#ifndef FHG_PLUGIN_CORE_KERNEL_HPP
#define FHG_PLUGIN_CORE_KERNEL_HPP 1

#include <string>
#include <list>

#include <boost/thread.hpp>

#include <fhg/util/thread/event.hpp>
#include <fhg/util/thread/queue.hpp>

#include <fhg/plugin/core/plugin.hpp>
#include <fhg/plugin/kernel.hpp>

namespace fhg
{
  namespace core
  {
    class wait_until_stopped
    {
    public:
      void wait();
      void stop();
      boost::function<void()> make_request_stop();

      fhg::util::thread::event<> _stop_requested;
      fhg::util::thread::event<> _stopped;
    };

    class kernel_t : public plugin::Kernel
    {
    public:
      typedef std::vector<std::string> search_path_t;

      kernel_t ( std::string const& name
               , fhg::core::kernel_t::search_path_t search_path
               , boost::function<void()> request_stop
               , std::map<std::string, std::string> config_variables
               );
      ~kernel_t ();

      boost::function<void()> _stop;
      virtual void stop() { _stop(); }

      int load_plugin (std::string const & entity);
      int load_plugin_by_name (std::string const & name);
      int load_plugin_from_file (std::string const & file);

      virtual std::string get(std::string const & key, std::string const &dflt) const;

      virtual std::string const & get_name () const;
    private:
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;
      typedef boost::condition_variable_any condition_type;

      typedef std::map<std::string, plugin_t::ptr_t> plugin_map_t;
      typedef std::map<std::string, std::string> config_t;

      mutable mutex_type m_mtx_plugins;
      mutable mutex_type m_mtx_load_plugin;

      plugin_map_t   m_plugins;
      std::list<std::string> m_load_order;

      config_t m_config;

      std::string m_name;
      search_path_t m_search_path;
    };
  }
}

#endif
