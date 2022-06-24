// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <iml/vmem/gaspi/gpi/gaspi.hpp>

#include <iml/vmem/gaspi/exception.hpp>

#include <util-generic/divru.hpp>
#include <util-generic/hostname.hpp>
#include <util-generic/syscall.hpp>

#include <GASPI.h>
#include <GASPI_Ext.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <numeric>
#include <unordered_set>

#include <unistd.h>

namespace gpi
{
  namespace api
  {
    namespace
    {
      [[noreturn]] void throw_gaspi_error
        ( std::string const& function_name
        , gaspi_return_t rc
        )
      {
        throw gpi::exception::gaspi_error
          ( gpi::error::internal_error()
          , function_name + " failed: " + gaspi_error_str (rc)
          + " (" + std::to_string (rc) + ")"
          );
      }

      template <typename Fun, typename... T>
      void fail_on_non_zero ( std::string const& function_name
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

    namespace
    {
      std::uint64_t get_total_memory_size()
      {
        auto const phys_pages (fhg::util::syscall::sysconf (_SC_PHYS_PAGES));
        auto const pagesize (fhg::util::syscall::sysconf (_SC_PAGESIZE));
        return phys_pages * pagesize;
      }
    }

    gaspi_t::gaspi_t ( fhg::iml::vmem::gaspi_context& gaspi_context
                     , unsigned long long per_node_size
                     , fhg::iml::vmem::gaspi_timeout& time_left
                     )
      : _gaspi_context (gaspi_context)
      , _per_node_size (per_node_size)
      , _segment_id (gaspi_context)
      , _notification_check_interrupted (false)
      , _interrupt_notification_check
          ([this] { _notification_check_interrupted = true; })
    {
      if (get_total_memory_size() < _per_node_size)
      {
        throw gpi::exception::gaspi_error
          ( gpi::error::startup_failed()
          , "not enough memory: requested memory size ("
          + std::to_string (_per_node_size) + ") exceeds total memory size ("
          + std::to_string (get_total_memory_size()) + ")"
          );
      }

      FAIL_ON_NON_ZERO ( gaspi_segment_create
                       , _segment_id
                       , _per_node_size
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
                   )
        //! \note only use even amounts (ping + pong ids)
        & ~notification_id_t (1);
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
          ( std::make_unique<decltype (_notification_check)::element_type>
              (&gaspi_t::notification_check, this)
          );
        std::swap (_notification_check, thread);
      }

      FAIL_ON_NON_ZERO (gaspi_barrier, GASPI_GROUP_ALL, time_left());
    }

    gaspi_t::~gaspi_t()
    {
      _notification_check_interrupted = true;
      _notification_check.reset();

      FAIL_ON_NON_ZERO (gaspi_segment_delete, _segment_id);
    }

    gpi::size_t gaspi_t::per_node_size() const
    {
      return _per_node_size;
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
      std::lock_guard<std::mutex> const _ (_queue_operation_guard);

      gaspi_number_t queue_size_max;
      gaspi_number_t queue_size;
      gaspi_number_t queue_num;

      FAIL_ON_NON_ZERO (gaspi_queue_size_max, &queue_size_max);
      FAIL_ON_NON_ZERO (gaspi_queue_size, _current_queue, &queue_size);
      FAIL_ON_NON_ZERO (gaspi_queue_num, &queue_num);

      assert (queue_entry_count <= queue_size_max);

      if ((queue_size + queue_entry_count) > queue_size_max)
      {
        _current_queue = (_current_queue + 1ul) % queue_num;

        FAIL_ON_NON_ZERO (gaspi_wait, _current_queue, GASPI_BLOCK);
      }

      FAIL_ON_NON_ZERO ( std::forward<Fun> (function)
                       , std::forward<Args> (arguments)...
                       , _current_queue
                       , GASPI_BLOCK
                       );

      return _current_queue;
    }

    gaspi_t::read_dma_info gaspi_t::read_dma (transfers_t const& transfers)
    {
      std::unordered_set<queue_desc_t> queues;

      for (auto const& transfer : transfers)
      {
        size_t remaining (transfer.size);
        const size_t chunk_size (_max_transfer_size);

        size_t l_off (transfer.local_offset);
        size_t r_off (transfer.remote_offset);

        while (remaining)
        {
          const size_t to_transfer (std::min (chunk_size, remaining));

          queues.emplace
            ( queued_operation<1> ( gaspi_read
                                  , _segment_id
                                  , l_off
                                  , transfer.rank
                                  , _segment_id
                                  , r_off
                                  , to_transfer
                                  )
            );

          remaining -= to_transfer;
          l_off     += to_transfer;
          r_off     += to_transfer;
        }
      }

      return {queues};
    }
    void gaspi_t::wait_readable (read_dma_info&& info)
    {
      //! \todo wait on specific reads instead (gaspi_read_notify)
      std::unordered_set<queue_desc_t> waited_queues;
      for (auto const& queue : info.queues)
      {
        if (waited_queues.emplace (queue).second)
        {
          FAIL_ON_NON_ZERO (gaspi_wait, queue, GASPI_BLOCK);
        }
      }
    }


    gaspi_t::write_dma_info gaspi_t::write_dma (transfers_t const& transfers)
    {
      notification_t const write_id (next_write_id());

      gpi::size_t const chunk_size (_max_transfer_size);
      std::size_t const chunks
        ( std::accumulate
            ( transfers.begin()
            , transfers.end()
            , std::size_t (0)
            , [&chunk_size] (std::size_t accum, transfer_part const& part)
              {
                return accum + fhg::util::divru (part.size, chunk_size);
              }
            )
        );

      {
        std::lock_guard<std::mutex> const _ (_notification_guard);
        if (!_outstanding_notifications.emplace (write_id, chunks).second)
        {
          throw std::logic_error
            ("write_id already exists in outstanding notifications");
        }
      }

      for (auto const& transfer : transfers)
      {
        size_t remaining (transfer.size);

        size_t l_off (transfer.local_offset);
        size_t r_off (transfer.remote_offset);

        while (remaining)
        {
          const size_t to_transfer (std::min (chunk_size, remaining));

          queued_operation<2> ( gaspi_write_notify
                              , _segment_id
                              , l_off
                              , transfer.rank
                              , _segment_id
                              , r_off
                              , to_transfer
                              , next_ping_id (transfer.rank)
                              , write_id
                              );

          remaining -= to_transfer;
          l_off     += to_transfer;
          r_off     += to_transfer;
        }
      }

      return {write_id};
    }

    void gaspi_t::wait_remote_written (write_dma_info&& info)
    {
      std::unique_lock<std::mutex> lock (_notification_guard);

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
      while (!_notification_check_interrupted)
      {
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
            std::lock_guard<std::mutex> const _ (_notification_guard);
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
