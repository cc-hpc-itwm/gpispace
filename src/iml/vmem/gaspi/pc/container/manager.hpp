// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <iml/MemorySize.hpp>
#include <iml/vmem/gaspi/pc/global/topology.hpp>
#include <iml/vmem/gaspi/pc/memory/manager.hpp>
#include <iml/vmem/gaspi/pc/type/impl_types.hpp>

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
        manager_t ( std::string const& p
                  , fhg::iml::vmem::gaspi_context&
                  , std::unique_ptr<fhg::rpc::service_tcp_provider_with_deferred_dispatcher> topology_rpc_server
                  );

        ~manager_t ();

      private:
        void listener_thread_main();
        void process_communication_thread (gpi::pc::type::process_id_t, int socket);

        void close_socket (int fd);
        void safe_unlink (std::string const& path);

        std::string m_path;
        int m_socket;
        bool m_stopping;

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
