#ifndef FHG_PLUGIN_CORE_KERNEL_HPP
#define FHG_PLUGIN_CORE_KERNEL_HPP 1

#include <string>
#include <list>

#include <boost/thread.hpp>

#include <fhg/util/thread/event.hpp>
#include <fhg/util/thread/queue.hpp>

#include <fhg/plugin/core/plugin.hpp>

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

    class kernel_t
    {
    public:
      typedef std::vector<std::string> search_path_t;

      kernel_t ( fhg::core::kernel_t::search_path_t search_path
               , boost::function<void()> request_stop
               , std::map<std::string, std::string> config_variables
               );
      ~kernel_t ();

      boost::function<void()> _stop;
      void stop() { _stop(); }

      int load_plugin_by_name (std::string const & name);
      int load_plugin_from_file (std::string const & file);

    private:
      std::map<std::string, plugin_t::ptr_t> m_plugins;
      std::list<std::string> m_load_order;

      std::map<std::string, std::string> m_config;

      search_path_t m_search_path;
    };
  }
}

#endif
