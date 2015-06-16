#pragma once

#include <fhglog/Logger.hpp>

#include <gpi-space/pc/global/topology.hpp>
#include <gpi-space/pc/memory/manager.hpp>
#include <gpi-space/pc/type/counter.hpp>
#include <gpi-space/pc/type/typedefs.hpp>

#include <boost/noncopyable.hpp>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace gpi
{
  namespace pc
  {
    namespace container
    {
      class manager_t : boost::noncopyable
      {
      public:
        manager_t ( fhg::log::Logger&
                  , std::string const & p
                  , std::vector<std::string> const& default_memory_urls
                  , api::gpi_api_t& gpi_api
                  , std::unique_ptr<fhg::com::peer_t> topology_peer
                    );

        ~manager_t ();

      private:
        void listener_thread_main();
        void process_communication_thread (gpi::pc::type::process_id_t, int socket);

        void close_socket (const int fd);
        void safe_unlink(std::string const & path);

        fhg::log::Logger& _logger;
        std::string m_path;
        int m_socket;
        bool m_stopping;

        gpi::pc::type::counter_t m_process_counter;
        mutable std::mutex _mutex_processes;
        std::map<gpi::pc::type::process_id_t, std::thread> m_processes;

        gpi::api::gpi_api_t& _gpi_api;
        memory::manager_t _memory_manager;
        global::topology_t _topology;

        std::thread _listener_thread;
      };
    }
  }
}
