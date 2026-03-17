// Copyright (C) 2011-2012,2014-2016,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/iml/vmem/gaspi/pc/global/topology.hpp>

#include <gspc/iml/vmem/gaspi/pc/memory/manager.hpp>

#include <gspc/rpc/remote_function.hpp>

#include <gspc/util/wait_and_collect_exceptions.hpp>

#include <boost/serialization/variant.hpp>

#include <future>



    namespace gpi::pc::global
    {
      topology_t::topology_t ( memory::manager_t& memory_manager
                             , gspc::iml::vmem::gaspi_context& gaspi_context
                             , std::unique_ptr<gspc::rpc::service_tcp_provider_with_deferred_dispatcher> server
                             )
        : _gaspi_context (gaspi_context)
        , _service_dispatcher()
        , _alloc ( _service_dispatcher
                 , [&memory_manager] ( gspc::iml::SegmentHandle seg
                                     , gspc::iml::AllocationHandle hdl
                                     , gspc::iml::MemoryOffset offset
                                     , gspc::iml::MemorySize size
                                     , gspc::iml::MemorySize local_size
                                     )
                   {
                     memory_manager.remote_alloc
                       (seg, hdl, offset, size, local_size);
                   }
                 , gspc::rpc::not_yielding
                 )
        , _free ( _service_dispatcher
                , [&memory_manager] (gspc::iml::AllocationHandle hdl)
                  {
                    memory_manager.remote_free (hdl);
                  }
                , gspc::rpc::not_yielding
                )
        , _add_memory ( _service_dispatcher
                      , [&memory_manager, this] ( gspc::iml::SegmentHandle seg_id
                                                , gspc::iml::SegmentDescription const& description
                                                , unsigned long total_size
                                                )
                        {
                          memory_manager.remote_add_memory
                            (seg_id, description, total_size, *this);
                        }
                      , gspc::rpc::not_yielding
                      )
        , _del_memory ( _service_dispatcher
                      , [&memory_manager, this] (gspc::iml::SegmentHandle seg_id)
                        {
                          memory_manager.remote_del_memory (seg_id, *this);
                        }
                      , gspc::rpc::not_yielding
                      )
        //! \todo count as parameter
        , _client_io_service (8)
        , _server (std::move (server))
      {
        _server->set_dispatcher (&_service_dispatcher);
      }

      namespace
      {
        template<typename Duration, typename ContainerOfFutures>
          void wait_for (ContainerOfFutures& futures, Duration duration)
        {
          auto const deadline
            (std::chrono::steady_clock::now() + duration);

          std::size_t fail_count (0);

          for (std::future<void>& future : futures)
          {
            if (future.wait_until (deadline) != std::future_status::ready)
            {
              ++fail_count;
            }
          }

          if (fail_count)
          {
            throw std::runtime_error ( std::to_string (fail_count)
                                     + " of "
                                     + std::to_string (futures.size())
                                     + " operations timed out"
                                     );
          }
        }
      }

      template<typename Description, typename... Args>
        void topology_t::request (Args&&... args)
      {
        //! \todo never store but just always create: calls are very rare
        if (_others.empty())
        {
          for (std::size_t rank (0); rank < _gaspi_context.number_of_nodes(); ++rank)
          {
            if (_gaspi_context.rank() == rank)
            {
              continue;
            }

            _others.emplace_back
              ( _client_io_service
              , _gaspi_context.hostname_of_rank (rank)
              , _gaspi_context.communication_port_of_rank (rank)
              );
          }
        }

        std::vector<std::future<void>> results;
        for (gspc::rpc::remote_tcp_endpoint& other : _others)
        {
          results.emplace_back
            (gspc::rpc::remote_function<Description> (other) (args...));
        }

        wait_for (results, std::chrono::seconds (30));
        gspc::util::wait_and_collect_exceptions (results);
      }

      void topology_t::free (gspc::iml::AllocationHandle hdl)
      {
        request<free_desc> (hdl);
      }

      void topology_t::alloc ( gspc::iml::SegmentHandle seg
                             , gspc::iml::AllocationHandle hdl
                             , gspc::iml::MemoryOffset offset
                             , gspc::iml::MemorySize size
                             , gspc::iml::MemorySize local_size
                             )
      {
        // lock, so that no other process can make a global alloc
        std::lock_guard<std::mutex> const _ (m_global_alloc_mutex);

        try
        {
          request<alloc_desc> (seg, hdl, offset, size, local_size);
        }
        catch (...)
        {
          free (hdl);
          throw;
        }
      }

      void topology_t::add_memory ( gspc::iml::SegmentHandle seg_id
                                  , gspc::iml::SegmentDescription const& description
                                  , unsigned long total_size
                                  )
      {
        request<add_memory_desc> (seg_id, description, total_size);
      }

      void topology_t::del_memory (gspc::iml::SegmentHandle seg_id)
      {
        request<del_memory_desc> (seg_id);
      }
    }
