#include <gpi-space/pc/global/topology.hpp>

#include <fhglog/LogMacros.hpp>

#include <gpi-space/gpi/gaspi.hpp>
#include <gpi-space/pc/memory/manager.hpp>

#include <util-generic/wait_and_collect_exceptions.hpp>

#include <future>

namespace gpi
{
  namespace pc
  {
    namespace global
    {
      topology_t::topology_t ( memory::manager_t& memory_manager
                             , api::gaspi_t& gaspi
                             , std::unique_ptr<fhg::rpc::server_with_multiple_clients_and_deferred_dispatcher> server
                             )
        : _gaspi (gaspi)
        , _service_dispatcher
            (fhg::util::serialization::exception::serialization_functions())
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
                 )
        , _free ( _service_dispatcher
                , [&memory_manager] (type::handle_t hdl)
                  {
                    memory_manager.remote_free (hdl);
                  }
                )
        , _add_memory ( _service_dispatcher
                      , [&memory_manager, this] ( type::segment_id_t seg_id
                                                , std::string url_s
                                                )
                        {
                          memory_manager.remote_add_memory
                            (seg_id, url_s, *this);
                        }
                      )
        , _del_memory ( _service_dispatcher
                      , [&memory_manager, this] (type::segment_id_t seg_id)
                        {
                          memory_manager.remote_del_memory (seg_id, *this);
                        }
                      )
        , _server (std::move (server))
      {
        _server->set_dispatcher (&_service_dispatcher);
      }

      bool topology_t::is_master () const
      {
        return 0 == _gaspi.rank();
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
          for (std::size_t rank (0); rank < _gaspi.number_of_nodes(); ++rank)
          {
            if (_gaspi.rank() == rank)
            {
              continue;
            }

            _others.emplace_back
              ( _gaspi.hostname_of_rank (rank)
              , _gaspi.communication_port_of_rank (rank)
              );
          }
        }

        //! \todo use aggregated_* rpc
        std::vector<std::future<void>> results;
        std::vector<fhg::rpc::remote_function<Description>> functions;
        for (fhg::rpc::remote_endpoint& other : _others)
        {
          functions.emplace_back (other);
          results.emplace_back (functions.back() (args...));
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
        std::unique_lock<std::mutex> const _ (m_global_alloc_mutex);

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
