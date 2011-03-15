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
          LOG(TRACE, "transfering data locally (memcpy)");
          task_list.push_back
            (boost::make_shared<task_t>
            ( "memcpy", boost::bind( &transfer_queue_t::do_memcpy
                                   , t
                                   )
            , t.amount
            ));
        }
        else if (t.dst_area->type() == t.src_area->type())
        {
          if (src_is_local)
          {
            task_list.push_back
              (boost::make_shared<task_t>
              ("writeDMA", boost::bind( &transfer_queue_t::do_write_dma
                                      , t
                                      )
              )
              );
          }
          else if (dst_is_local)
          {
            task_list.push_back
              (boost::make_shared<task_t>
              ("readDMA", boost::bind( &transfer_queue_t::do_read_dma
                                      , t
                                      )
              )
              );
          }
          else
          {
            throw std::runtime_error
              ( "illegal memory transfer requested:"
              " source and destination cannot both be remote"
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
      transfer_queue_t::do_memcpy (memory_transfer_t t)
      {
        LOG(TRACE, "memcpy: " << t);

        std::memcpy( t.dst_area->pointer_to(t.dst_location)
                   , t.src_area->pointer_to(t.src_location)
                   , t.amount
                   );
      }

      void
      transfer_queue_t::do_read_dma (memory_transfer_t t)
      {
        LOG(TRACE, "gpi::readDMA: " << t);

        // HACK
        if (t.src_area->type() == gpi_area_t::area_type)
        {
          gpi_area_t *area(static_cast<gpi_area_t*>(t.src_area.get()));
          area->read_dma (t.src_location, t.dst_location, t.amount, t.queue);
        }
        else
        {
          CLOG( ERROR
              , "gpi.memory"
              , "RDMA is not yet implemented for area type " << t.src_area->type()
              );
          throw std::runtime_error
            ("RDMA is not yet supported for this memory type!");
        }
      }

      void
      transfer_queue_t::do_write_dma (memory_transfer_t t)
      {
        LOG(TRACE, "gpi::writeDMA: " << t);

        // HACK
        if (t.src_area->type() == gpi_area_t::area_type)
        {
          gpi_area_t *area(static_cast<gpi_area_t*>(t.src_area.get()));
          area->write_dma (t.dst_location, t.src_location, t.amount, t.queue);
        }
        else
        {
          CLOG( ERROR
              , "gpi.memory"
              , "RDMA is not yet implemented for area type " << t.src_area->type()
              );
          throw std::runtime_error
            ("RDMA is not yet supported for this memory type!");
        }
      }

      void
      transfer_queue_t::do_wait_on_queue (const std::size_t q)
      {
        DLOG(TRACE, "gpi::wait_dma(" << q << ")");
        std::size_t s(gpi::api::gpi_api_t::get().wait_dma(q));
        DLOG(TRACE, "gpi::wait_dma(" << q << ") = " << s);
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
        task_ptr wtask (boost::make_shared<task_t>
                       ("wait_on_queue", boost::bind( &transfer_queue_t::do_wait_on_queue
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
          (*wait_on_tasks.begin())->wait();
          wait_on_tasks.erase(wait_on_tasks.begin());
        }
        return res;
      }
    }
  }
}
