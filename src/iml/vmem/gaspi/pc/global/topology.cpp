#include <iml/vmem/gaspi/pc/global/topology.hpp>

#include <iml/vmem/gaspi/pc/memory/manager.hpp>

#include <rpc/remote_function.hpp>

#include <util-generic/wait_and_collect_exceptions.hpp>

#include <future>

namespace gpi
{
  namespace pc
  {
    namespace global
    {
      topology_t::topology_t ( memory::manager_t& memory_manager
                             , fhg::iml::vmem::gaspi_context& gaspi_context
                             , std::unique_ptr<fhg::rpc::service_tcp_provider_with_deferred_dispatcher> server
                             )
        : _gaspi_context (gaspi_context)
        , _service_dispatcher()
        , _alloc ( _service_dispatcher
                 , [&memory_manager] ( type::segment_id_t seg
                                     , type::handle_t hdl
                                     , type::offset_t offset
                                     , type::size_t size
                                     , type::size_t local_size
                                     , std::string name
                                     )
                   {
                     memory_manager.remote_alloc
                       (seg, hdl, offset, size, local_size, name);
                   }
                 , fhg::rpc::not_yielding
                 )
        , _free ( _service_dispatcher
                , [&memory_manager] (type::handle_t hdl)
                  {
                    memory_manager.remote_free (hdl);
                  }
                , fhg::rpc::not_yielding
                )
        , _add_memory ( _service_dispatcher
                      , [&memory_manager, this] ( type::segment_id_t seg_id
                                                , std::string url_s
                                                )
                        {
                          memory_manager.remote_add_memory
                            (seg_id, url_s, *this);
                        }
                      , fhg::rpc::not_yielding
                      )
        , _del_memory ( _service_dispatcher
                      , [&memory_manager, this] (type::segment_id_t seg_id)
                        {
                          memory_manager.remote_del_memory (seg_id, *this);
                        }
                      , fhg::rpc::not_yielding
                      )
        //! \todo count as parameter
        , _client_io_service (8)
        , _server (std::move (server))
      {
        _server->set_dispatcher (&_service_dispatcher);
      }

      bool topology_t::is_master () const
      {
        return 0 == _gaspi_context.rank();
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
        for (fhg::rpc::remote_tcp_endpoint& other : _others)
        {
          results.emplace_back
            (fhg::rpc::remote_function<Description> (other) (args...));
        }

        wait_for (results, std::chrono::seconds (30));
        fhg::util::wait_and_collect_exceptions (results);
      }

      void topology_t::free (const gpi::pc::type::handle_t hdl)
      {
        request<free_desc> (hdl);
      }

      void topology_t::alloc ( const gpi::pc::type::segment_id_t seg
                             , const gpi::pc::type::handle_t hdl
                             , const gpi::pc::type::offset_t offset
                             , const gpi::pc::type::size_t size
                             , const gpi::pc::type::size_t local_size
                             , const std::string & name
                             )
      {
        // lock, so that no other process can make a global alloc
        std::lock_guard<std::mutex> const _ (m_global_alloc_mutex);

        try
        {
          request<alloc_desc> (seg, hdl, offset, size, local_size, name);
        }
        catch (...)
        {
          free (hdl);
          throw;
        }
      }

      void topology_t::add_memory ( const gpi::pc::type::segment_id_t seg_id
                                  , const std::string & url
                                  )
      {
        request<add_memory_desc> (seg_id, url);
      }

      void topology_t::del_memory (const gpi::pc::type::segment_id_t seg_id)
      {
        request<del_memory_desc> (seg_id);
      }
    }
  }
}
