#include "transfer_queue.hpp"
#include "gpi_area.hpp"

#include <fhglog/minimal.hpp>

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include <gpi-space/gpi/api.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      transfer_queue_t::transfer_queue_t ( const std::size_t id
                                         , task_queue_t *queue_to_pool
                                         )
        : m_id (id)
        , m_paused (false)
        , m_blocking_tasks (*queue_to_pool)
      {
        m_thread = boost::make_shared<boost::thread>
          (boost::bind ( &transfer_queue_t::worker
                       , this
                       )
          );
      }

      transfer_queue_t::~transfer_queue_t ()
      {
        // clear all pending and all finished
        m_thread->interrupt ();
        m_thread->join ();
      }

      void
      transfer_queue_t::worker ()
      {
        for (;;)
        {
          task_ptr task = m_task_queue.pop();
          task->execute ();
          if (task->has_failed())
          {
            LOG( ERROR
               , "task failed: " << task->get_name() << ": "
               << task->get_error_message()
               );
          }
          else if (task->has_finished())
          {
            DLOG(TRACE, "transfer done: " << task->get_name());
          }
          else
          {
            LOG( ERROR
               , "*** STRANGE: task neither finished, nor failed, but did return?"
               << " task := " << task->get_name()
               );
          }
        }
      }

      void
      transfer_queue_t::enqueue (const memory_transfer_t &t)
      {
        wait_until_unpaused ();
        enqueue(split(t));
      }

      void
      transfer_queue_t::enqueue (const task_list_t & tasks)
      {
        static const std::size_t delegate_threshold(0);

        // this  needs to  be atomic,  otherwise (enqueue();  wait();)  would be
        // broken
        {
          lock_type lock (m_mutex);
          m_dispatched.insert (tasks.begin(), tasks.end());
        }

        BOOST_FOREACH(task_ptr task, tasks)
        {
          if (task->time_estimation() > delegate_threshold)
          {
            m_blocking_tasks.push (task);
          }
          else
          {
            m_task_queue.push(task);
          }
        }
      }

      static void do_memcpy ( memory_transfer_t t
                            , const std::size_t chunk
                            , const std::size_t shift
                            )
      {
        DLOG( TRACE
            , "memcpy:"
            << " process " << t.pid
            << " copies " << chunk << " bytes"
            << " via " << t.queue
            << " from " << t.src_location
            << " to " << t.dst_location
            << " shifted by " << shift
            );

        std::memcpy( (char*)(t.dst_area->pointer_to(t.dst_location)) + shift
                   , (char*)(t.src_area->pointer_to(t.src_location)) + shift
                   , chunk
                   );
      }

      static
      void do_wait_on_queue (const std::size_t q)
      {
        DLOG(TRACE, "gpi::wait_dma(" << q << ")");
        std::size_t s(gpi::api::gpi_api_t::get().wait_dma(q));
        DLOG(TRACE, "gpi::wait_dma(" << q << ") = " << s);
      }


      static
      void do_read_dma_gpi ( const gpi::pc::type::offset_t local_offset
                           , const gpi::pc::type::offset_t remote_offset
                           , const gpi::pc::type::size_t amount
                           , const gpi::pc::type::rank_t from_node
                           , const gpi::pc::type::queue_id_t queue
                           )
      {
        DLOG( TRACE, "read_dma:"
            << " loc-offset = " << local_offset
            << " rem-offset = " << remote_offset
            << " #bytes = " << amount
            << " from = " << from_node
            << " via = " << queue
            );

        gpi::api::gpi_api_t & api = gpi::api::gpi_api_t::get();

//         if (api.rank() == from_node)
//         {
//           DLOG(INFO, "readDMA from local node to local node");
//         }

        if (api.max_dma_requests_reached(queue))
        {
          do_wait_on_queue(queue);
        }

        api.read_dma (local_offset, remote_offset, amount, from_node, queue);
      }

      template <typename DMAFun>
      static
      void do_rdma( gpi::pc::type::size_t local_offset
                  , const gpi::pc::type::size_t remote_base
                  , gpi::pc::type::size_t offset
                  , const gpi::pc::type::size_t per_node_size
                  , gpi::pc::type::size_t amount
                  , const gpi::pc::type::queue_id_t queue
                  , DMAFun rdma
                  )
      {
        while (amount)
        {
          const std::size_t rank(offset / per_node_size);
          const std::size_t max_offset_on_rank ((rank + 1) * per_node_size);
          const std::size_t size(std::min ( std::min(per_node_size, amount)
                                          , max_offset_on_rank - offset
                                          )
                                );
          const std::size_t remote_offset(remote_base + (offset % per_node_size));

          rdma(local_offset, remote_offset, size, rank, queue);

          local_offset += size;
          offset += size;
          amount -= size;
        }
      }


      static
      void do_read_dma (memory_transfer_t t)
      {
        const gpi::pc::type::handle::descriptor_t src_hdl
          (t.src_area->descriptor(t.src_location.handle));
        const gpi::pc::type::handle::descriptor_t dst_hdl
          (t.dst_area->descriptor(t.dst_location.handle));

        do_rdma( dst_hdl.offset + (t.dst_location.offset % dst_hdl.size)
               , src_hdl.offset , t.src_location.offset
               , src_hdl.size
               , t.amount
               , t.queue
               , &do_read_dma_gpi
               );
      }

      static
      void do_write_dma_gpi ( const gpi::pc::type::offset_t local_offset
                            , const gpi::pc::type::offset_t remote_offset
                            , const gpi::pc::type::size_t amount
                            , const gpi::pc::type::rank_t to_node
                            , const gpi::pc::type::queue_id_t queue
                            )
      {
        DLOG( TRACE, "write_dma:"
            << " loc-offset = " << local_offset
            << " rem-offset = " << remote_offset
            << " #bytes = " << amount
            << " to = " << to_node
            << " via = " << queue
            );

        gpi::api::gpi_api_t & api = gpi::api::gpi_api_t::get();

//         if (api.rank() == to_node)
//         {
//           LOG(INFO, "writeDMA to local node from local node");
//         }

        if (api.max_dma_requests_reached(queue))
        {
          do_wait_on_queue(queue);
        }

        api.write_dma (local_offset, remote_offset, amount, to_node, queue);
      }

      static
      void do_write_dma (memory_transfer_t t)
      {
        const gpi::pc::type::handle::descriptor_t src_hdl
          (t.src_area->descriptor(t.src_location.handle));
        const gpi::pc::type::handle::descriptor_t dst_hdl
          (t.dst_area->descriptor(t.dst_location.handle));

        do_rdma( src_hdl.offset + (t.src_location.offset % src_hdl.size)
               , dst_hdl.offset , t.dst_location.offset
               , dst_hdl.size
               , t.amount
               , t.queue
               , &do_write_dma_gpi
               );
      }

      static
      void fill_memcpy_list( const memory_transfer_t &t
                           , transfer_queue_t::task_list_t & task_list
                           )
      {
        DLOG(TRACE, "transfering data locally (memcpy)");

        const std::size_t chunk_size = t.amount;
        std::size_t remaining(t.amount);
        std::size_t shift (0);

        while (remaining)
        {
          std::size_t sz (std::min(remaining, chunk_size));
          task_list.push_back
            (boost::make_shared<task_t>
            ( "memcpy " + boost::lexical_cast<std::string>(t)
            , boost::bind( &do_memcpy
                         , t
                         , sz
                         , shift
                         )
            , sz
            ));
          remaining -= sz;
          shift += sz;
        }
      }

      static
      void fill_read_dma_list( const memory_transfer_t &t
                             , transfer_queue_t::task_list_t & task_list
                             )
      {
        task_list.push_back
          (boost::make_shared<task_t>
            ("readDMA", boost::bind( &do_read_dma
                                   , t
                                   )));
      }

      static
      void fill_write_dma_list( const memory_transfer_t &t
                              , transfer_queue_t::task_list_t & task_list
                              )
      {
        task_list.push_back
          (boost::make_shared<task_t>
            ("writeDMA", boost::bind( &do_write_dma
                                    , t
                                    )));
      }

      transfer_queue_t::task_list_t
      transfer_queue_t::split(const memory_transfer_t &t)
      {
        task_list_t task_list;

        const bool src_is_local
          (t.src_area->is_local(gpi::pc::type::memory_region_t( t.src_location
                                                              , t.amount
                                                              )
                               )
          );
        const bool dst_is_local
          (t.dst_area->is_local(gpi::pc::type::memory_region_t( t.dst_location
                                                              , t.amount
                                                              )
                               )
          );

        if (src_is_local && dst_is_local)
        {
          fill_memcpy_list(t, task_list);
        }
        else if (t.dst_area->type() == t.src_area->type())
        {
          if (src_is_local)
          {
            fill_write_dma_list(t, task_list);
          }
          else if (dst_is_local)
          {
            fill_read_dma_list(t, task_list);
          }
          else
          {
            throw std::runtime_error
              ( "illegal memory transfer requested:"
              " source and destination cannot both be remote!"
              );
          }
        }
        else
        {
          throw std::runtime_error
            ( "illegal memory transfer requested:"
            " I have no idea how to transfer data between those segments, sorry!"
            );
        }
        return task_list;
      }

      void
      transfer_queue_t::wait_until_unpaused () const
      {
        lock_type lock (m_mutex);
        while (is_paused())
        {
          m_resume_condition.wait (lock);
        }
      }

      void
      transfer_queue_t::resume ()
      {
        lock_type lock (m_mutex);
        m_paused = false;
        m_resume_condition.notify_all();
      }

      void
      transfer_queue_t::pause ()
      {
        lock_type lock (m_mutex);
        m_paused = true;
      }

      bool
      transfer_queue_t::is_paused () const
      {
        return m_paused;
      }

      std::size_t
      transfer_queue_t::wait ()
      {
        lock_type global_wait_lock (m_global_wait_mutex);

        task_ptr wtask (boost::make_shared<task_t>
                       ("wait_on_queue", boost::bind( &do_wait_on_queue
                                                    , m_id
                                                    )
                       )
                       );
        m_task_queue.push (wtask);
        wtask->wait ();

        task_set_t wait_on_tasks;
        {
          lock_type lock (m_mutex);
          m_dispatched.swap(wait_on_tasks);
        }

        std::size_t res(wait_on_tasks.size());
        while (! wait_on_tasks.empty())
        {
          task_ptr task(*wait_on_tasks.begin());
          wait_on_tasks.erase(wait_on_tasks.begin());
          task->wait();

          // TODO: WORK HERE:  this failure should be propagated  to the correct
          // process container
          if (task->has_failed())
          {
            LOG( ERROR
               , "transfer " << task->get_name()
               << " failed: " << task->get_error_message()
               );
          }
        }
        return res;
      }
    }
  }
}
