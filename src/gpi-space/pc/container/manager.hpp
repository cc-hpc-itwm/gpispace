#pragma once

#include <fhglog/Logger.hpp>

#include <gpi-space/pc/global/topology.hpp>
#include <gpi-space/pc/memory/manager.hpp>
#include <gpi-space/pc/type/typedefs.hpp>

#include <boost/noncopyable.hpp>

#include <atomic>
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
                  , std::unique_ptr<fhg::rpc::server_with_multiple_clients_and_deferred_dispatcher> topology_rpc_server
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

        std::atomic<gpi::pc::type::size_t> m_process_counter;
        mutable std::mutex _mutex_processes;
        std::map<gpi::pc::type::process_id_t, std::thread> m_processes;

        memory::manager_t _memory_manager;
        global::topology_t _topology;

        std::thread _listener_thread;
      };
    }
  }
}
