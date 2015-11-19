#include <gpi-space/gpi/gaspi.hpp>

#include <fhglog/LogMacros.hpp>
#include <fhg/assert.hpp>

#include <gpi-space/exception.hpp>
#include <gpi-space/gpi/system.hpp>

#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/divru.hpp>
#include <util-generic/hostname.hpp>

#include <GASPI.h>

#include <algorithm>
#include <limits>
#include <unordered_set>

namespace gpi
{
  namespace api
  {
    namespace
    {
      void throw_gaspi_error ( std::string const& function_name
                             , gaspi_return_t rc
                             )
      {
        throw gpi::exception::gpi_error
          ( gpi::error::internal_error()
          , function_name + " failed: " + gaspi_error_str (rc)
          + " (" + std::to_string (rc) + ")"
          );
      }

      template <typename Fun, typename... T>
      void fail_on_non_zero ( const std::string& function_name
                            , Fun&& f
                            , T&&... arguments
                            )
      {
        gaspi_return_t const rc (f (arguments...));
        if (rc != 0)
        {
          throw_gaspi_error (function_name, rc);
        }
      }

#define FAIL_ON_NON_ZERO(F, Args...)             \
      fail_on_non_zero(#F, F, Args)
    }

    gaspi_t::gaspi_t ( fhg::vmem::gaspi_context& gaspi_context
                     , fhg::log::Logger& logger
                     , const unsigned long long memory_size
                     , fhg::vmem::gaspi_timeout& time_left
                     )
      : _gaspi_context (gaspi_context)
      , _logger (logger)
      , m_mem_size (memory_size)
      , m_dma (nullptr)
      , _segment_id (gaspi_context)
      , _current_queue (0)
    {
      if (sys::get_total_memory_size() < m_mem_size)
      {
        throw gpi::exception::gpi_error
          ( gpi::error::startup_failed()
          , "not enough memory: requested memory size ("
          + std::to_string (m_mem_size) + ") exceeds total memory size ("
          + std::to_string (sys::get_total_memory_size()) + ")"
          );
      }
      else if (sys::get_avail_memory_size() < m_mem_size)
      {
        LLOG( WARN
            , _logger
           , "requested memory size (" << m_mem_size << ")"
           <<" exceeds available memory size (" << sys::get_avail_memory_size() << ")"
           );
      }

      FAIL_ON_NON_ZERO ( gaspi_segment_create
                       , _segment_id
                       , m_mem_size
                       , GASPI_GROUP_ALL
                       , time_left()
                       , GASPI_MEM_UNINITIALIZED
                       );
      FAIL_ON_NON_ZERO ( gaspi_segment_ptr
                       , _segment_id
                       , &m_dma
                       );

      FAIL_ON_NON_ZERO (gaspi_transfer_size_max, &_max_transfer_size);

      gaspi_number_t available_notifications;
      FAIL_ON_NON_ZERO (gaspi_notification_num, &available_notifications);

      //! \note limited due to rui.machado/gpi-2#10: notification
      //! values are not written atomically in gpi-2+eth but on a
      //! byte-by-byte level, thus we limit us to using just a single
      //! byte at any given time.
      {
        std::vector<gaspi_notification_t> write_ids;

        constexpr std::size_t const bytes (sizeof (gaspi_notification_t));
        //! \note do not use 0 as notification value
        constexpr std::size_t const ids_per_byte ((1 << CHAR_BIT) - 1);
        for (gaspi_notification_t n (1); n < ids_per_byte; ++n)
        {
          for (std::size_t byte (0); byte < bytes; ++byte)
          {
            write_ids.emplace_back (n << (byte * CHAR_BIT));
          }
        }

        _write_ids.put_many (write_ids.begin(), write_ids.end());
      }

      //! \todo reasonable maximum
      constexpr gaspi_number_t const maximum_notifications_per_rank (16);

      _notification_ids_per_node
        = std::min ( available_notifications
                   / gaspi_number_t (_gaspi_context.number_of_nodes())
                   , maximum_notifications_per_rank
                   );
      if (_notification_ids_per_node < 2)
      {
        throw std::runtime_error
          ( "need at least two notification ids per rank ("
          + std::to_string (available_notifications)
          + " notification ids available in total, but "
          + std::to_string (_gaspi_context.number_of_nodes()) + " ranks)"
          );
      }

      {
        std::vector<gaspi_notification_id_t> ping_ids (ping_ids_count());
        std::iota (ping_ids.begin(), ping_ids.end(), ids_begin (_gaspi_context.rank()));
        for (gaspi_rank_t rank (0); rank < _gaspi_context.number_of_nodes(); ++rank)
        {
          _ping_ids[rank].put_many (ping_ids.begin(), ping_ids.end());
        }
      }

      {
        auto thread
          ( fhg::util::cxx14::make_unique<decltype (_notification_check)::element_type>
              (&gaspi_t::notification_check, this)
          );
        std::swap (_notification_check, thread);
      }

      FAIL_ON_NON_ZERO (gaspi_barrier, GASPI_GROUP_ALL, time_left());
    }

    gaspi_t::~gaspi_t()
    {
      _notification_check.reset();

      FAIL_ON_NON_ZERO (gaspi_segment_delete, _segment_id);
    }

    gpi::size_t gaspi_t::memory_size() const
    {
      return m_mem_size;
    }

    void* gaspi_t::dma_ptr() const
    {
      return m_dma;
    }

    template<std::size_t queue_entry_count, typename Fun, typename... Args>
      queue_desc_t gaspi_t::queued_operation ( Fun&& function
                                             , Args&&... arguments
                                             )
    {
      //! \todo do without synchronization by knowing the number of
      //! threads and thus how many queue entries can be taken if a
      //! thread gets suspended and woken up again between the queue
      //! entry check and the actual operation.
      std::unique_lock<std::mutex> const _ (_queue_operation_guard);

      gaspi_number_t queue_size_max;
      gaspi_number_t queue_size;
      gaspi_number_t queue_num;

      FAIL_ON_NON_ZERO (gaspi_queue_size_max, &queue_size_max);
      FAIL_ON_NON_ZERO (gaspi_queue_size, _current_queue, &queue_size);
      FAIL_ON_NON_ZERO (gaspi_queue_num, &queue_num);

      assert (queue_entry_count <= queue_size_max);

      if ((queue_size + queue_entry_count) > queue_size_max)
      {
        _current_queue = (_current_queue + 1) % queue_num;

        FAIL_ON_NON_ZERO (gaspi_wait, _current_queue, GASPI_BLOCK);
      }

      FAIL_ON_NON_ZERO ( std::forward<Fun> (function)
                       , std::forward<Args> (arguments)...
                       , _current_queue
                       , GASPI_BLOCK
                       );

      return _current_queue;
    }

    gaspi_t::read_dma_info gaspi_t::read_dma
      ( const offset_t local_offset
      , const offset_t remote_offset
      , const size_t amount
      , const rank_t from_node
      )
    {
      std::unordered_set<queue_desc_t> queues;

      size_t remaining (amount);
      const size_t chunk_size (_max_transfer_size);

      size_t l_off (local_offset);
      size_t r_off (remote_offset);

      while (remaining)
      {
        const size_t to_transfer (std::min (chunk_size, remaining));

        queues.emplace
          ( queued_operation<1> ( gaspi_read
                                , _segment_id
                                , l_off
                                , from_node
                                , _segment_id
                                , r_off
                                , to_transfer
                                )
          );

        remaining -= to_transfer;
        l_off     += to_transfer;
        r_off     += to_transfer;
      }

      return {queues};
    }
    void gaspi_t::wait_readable (std::list<read_dma_info> const& infos)
    {
      //! \todo wait on specific reads instead (gaspi_read_notify)
      std::unordered_set<queue_desc_t> waited_queues;
      for (read_dma_info const& info : infos)
      {
        for (auto const& queue : info.queues)
        {
          if (waited_queues.emplace (queue).second)
          {
            FAIL_ON_NON_ZERO (gaspi_wait, queue, GASPI_BLOCK);
          }
        }
      }
    }

    gaspi_t::write_dma_info gaspi_t::write_dma
      ( const offset_t local_offset
      , const offset_t remote_offset
      , const size_t amount
      , const rank_t to_node
      )
    {
      size_t remaining (amount);
      const size_t chunk_size (_max_transfer_size);

      size_t l_off (local_offset);
      size_t r_off (remote_offset);

      notification_t const write_id (next_write_id());
      std::size_t const chunks (fhg::util::divru (amount, chunk_size));

      {
        std::unique_lock<std::mutex> const _ (_notification_guard);
        if (!_outstanding_notifications.emplace (write_id, chunks).second)
        {
          throw std::logic_error
            ("write_id already exists in outstanding notifications");
        }
      }

      while (remaining)
      {
        const size_t to_transfer (std::min (chunk_size, remaining));

        queued_operation<2> ( gaspi_write_notify
                            , _segment_id
                            , l_off
                            , to_node
                            , _segment_id
                            , r_off
                            , to_transfer
                            , next_ping_id (to_node)
                            , write_id
                            );

        remaining -= to_transfer;
        l_off     += to_transfer;
        r_off     += to_transfer;
      }

      return {write_id};
    }

    void gaspi_t::wait_remote_written
      (std::list<write_dma_info> const& infos)
    {
      std::unordered_set<notification_t> waited_write_ids;

      std::unique_lock<std::mutex> lock (_notification_guard);

      for (write_dma_info const& info : infos)
      {
        if (waited_write_ids.emplace (info.write_id).second)
        {
          _notification_received.wait
            ( lock
            , [&]
              {
                return _outstanding_notifications.at (info.write_id) == 0;
              }
            );

          _outstanding_notifications.erase (info.write_id);
          _write_ids.put (info.write_id);
        }
      }
    }

    notification_id_t gaspi_t::pong_ids_offset() const
    {
      return _notification_ids_per_node / 2;
    }
    notification_id_t gaspi_t::ping_ids_count() const
    {
      return pong_ids_offset();
    }
    notification_id_t gaspi_t::total_number_of_notifications() const
    {
      return _notification_ids_per_node * _gaspi_context.number_of_nodes();
    }

    rank_t gaspi_t::sending_rank (notification_id_t notification_id) const
    {
      return notification_id / _notification_ids_per_node;
    }
    notification_id_t gaspi_t::ids_begin (rank_t rank) const
    {
      return rank * _notification_ids_per_node;
    }
    notification_id_t gaspi_t::notification_id_offset
      (notification_id_t notification_id) const
    {
      return notification_id - ids_begin (sending_rank (notification_id));
    }
    bool gaspi_t::is_pong (notification_id_t notification_id) const
    {
      return notification_id_offset (notification_id) >= pong_ids_offset();
    }
    notification_id_t gaspi_t::corresponding_local_ping_id
      (notification_id_t pong_id) const
    {
      return ids_begin (_gaspi_context.rank())
        + notification_id_offset (pong_id)
        - pong_ids_offset();
    }
    notification_id_t gaspi_t::corresponding_local_pong_id
      (notification_id_t ping_id) const
    {
      return ids_begin (_gaspi_context.rank())
        + notification_id_offset (ping_id)
        + pong_ids_offset();
    }

    void gaspi_t::notification_check()
    {
      for (;;)
      {
        boost::this_thread::interruption_point();

        gaspi_notification_id_t notification_id;
        gaspi_return_t const waitsome_result
          ( gaspi_notify_waitsome
              ( _segment_id
              , 0
              , total_number_of_notifications()
              , &notification_id
              , 100
              )
          );

        if (waitsome_result == GASPI_TIMEOUT)
        {
          continue;
        }
        else if (waitsome_result != GASPI_SUCCESS)
        {
          throw_gaspi_error
            ("notification_check: waitsome", waitsome_result);
        }

        gaspi_notification_t write_id;
        FAIL_ON_NON_ZERO ( gaspi_notify_reset
                         , _segment_id
                         , notification_id
                         , &write_id
                         );

        if (is_pong (notification_id))
        {
          {
            std::unique_lock<std::mutex> const _ (_notification_guard);
            --_outstanding_notifications.at (write_id);
          }

          _ping_ids[sending_rank (notification_id)].put
            (corresponding_local_ping_id (notification_id));
          _notification_received.notify_all();
        }
        else
        {
          queued_operation<1>
            ( gaspi_notify
            , _segment_id
            , sending_rank (notification_id)
            , corresponding_local_pong_id (notification_id)
            , write_id
            );
        }
      }
    }

    notification_t gaspi_t::next_write_id()
    {
      return _write_ids.get();
    }
    notification_id_t gaspi_t::next_ping_id (rank_t rank)
    {
      return _ping_ids[rank].get();
    }

#undef FAIL_ON_NON_ZERO
  }
}
