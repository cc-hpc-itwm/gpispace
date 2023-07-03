// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/MemorySize.hpp>
#include <iml/vmem/gaspi/pc/global/topology.hpp>
#include <iml/vmem/gaspi/pc/memory/manager.hpp>
#include <iml/vmem/gaspi/pc/type/impl_types.hpp>

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
      class manager_t
      {
      public:
        manager_t ( std::string const& p
                  , fhg::iml::vmem::gaspi_context&
                  , std::unique_ptr<fhg::rpc::service_tcp_provider_with_deferred_dispatcher> topology_rpc_server
                  );

        ~manager_t ();
        manager_t (manager_t const&) = delete;
        manager_t (manager_t&&) = delete;
        manager_t& operator= (manager_t const&) = delete;
        manager_t& operator= (manager_t&&) = delete;

      private:
        void listener_thread_main();
        void process_communication_thread (gpi::pc::type::process_id_t, int socket);

        void close_socket (int fd);
        void safe_unlink (std::string const& path);

        std::string m_path;
        int m_socket {-1};
        bool m_stopping {false};

        std::atomic<iml::MemorySize> m_process_counter;
        std::mutex _mutex_processes;
        std::map<gpi::pc::type::process_id_t, std::thread> m_processes;

        memory::manager_t _memory_manager;
        global::topology_t _topology;

        std::thread _listener_thread;
      };
    }
  }
}
