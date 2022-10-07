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

#pragma once

#include <iml/vmem/gaspi/types.hpp>

#include <iml/vmem/gaspi_context.hpp>

#include <util-generic/finally.hpp>
#include <util-generic/threadsafe_queue.hpp>

#include <boost/thread/scoped_thread.hpp>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace gpi
{
  namespace api
  {
    class gaspi_t
    {
    public:
      gaspi_t ( fhg::iml::vmem::gaspi_context&
              , unsigned long long per_node_size
              , fhg::iml::vmem::gaspi_timeout&
              );
      ~gaspi_t();
      gaspi_t (gaspi_t const&) = delete;
      gaspi_t (gaspi_t&&) = delete;
      gaspi_t& operator= (gaspi_t const&) = delete;
      gaspi_t& operator= (gaspi_t&&) = delete;

      gpi::size_t per_node_size() const;

      void* dma_ptr() const;

      struct transfer_part
      {
        offset_t local_offset;
        offset_t remote_offset;
        size_t size;
        rank_t rank;
      };
      using transfers_t = std::list<transfer_part>;

      struct read_dma_info
      {
        std::unordered_set<queue_desc_t> queues;
      };

      read_dma_info read_dma (transfers_t const&);
      void wait_readable (read_dma_info&&);

      struct write_dma_info
      {
        notification_t write_id;
      };

      write_dma_info write_dma (transfers_t const&);
      void wait_remote_written (write_dma_info&&);

    private:
      template<std::size_t queue_entry_count, typename Fun, typename... Args>
        queue_desc_t queued_operation (Fun&&, Args&&...);

      fhg::iml::vmem::gaspi_context& _gaspi_context;

      size_t _per_node_size;
      void *m_dma {nullptr};
      fhg::iml::vmem::gaspi_context::reserved_segment_id _segment_id;
      gpi::size_t _max_transfer_size;

      std::vector<std::string> m_rank_to_hostname;
      std::vector<unsigned short> _communication_port_by_rank;

      std::mutex _queue_operation_guard;
      queue_desc_t _current_queue {0};

      //! \note We split (1) the available notification
      //! (_maximum_notification_id) ids into $num_proc parts
      //! (_notification_ids_per_node) and split (2) them in half at
      //! (_pong_ids_offset) to have two blocks: ids to send pings and
      //! ids to send pongs, unique from each other to not
      //! overwrite. The ids to send pings are then pushed into
      //! _ping_ids to have a queue of ids not yet sent to any
      //! possible rank.
      //!
      //! ids      0 1 2 3 4 5 6 7 8 9 10 11
      //! ranks    0------ 1------ 2--------
      //! ping     x x     x x     x x
      //! pong         x x     x x     x  x
      //! _notification_ids_per_node = 4
      //! _pong_ids_offset = 2
      //!
      //! When writing we get a write id (next_write_id()) and the
      //! next not-yet-sent notification id is pulled from _ping_ids
      //! (next_ping_id()). The write id is stored with the number of
      //! chunks written in _outstanding_notifications. The
      //! notification_check() thread waitsome()s on all possible
      //! notifications. Incoming notifications get their sending rank
      //! identified (split 1) and the type (ping or pong) is
      //! determined by their offset in the given range of
      //! notification ids (split 2). Pings are answered with an
      //! receiving rank's notification id (thus uniquely identifying
      //! the initial sender's _ping_ids queue) with the same offset
      //! (thus uniquely identifying the notification id to put back
      //! into the queue). Pongs are decrementing the number of
      //! outstanding notifications for the write and put back the
      //! notification id.
      //!
      //! r1      : pick notification from queue of rank 2: 5
      //! r1 -> r2: notification 5
      //!       r2: rank = 5 / pn = 1. 5 - (rank * pn) = 1
      //! r1 <- r2: notification local_ids_begin + 1 + pong_ids_offset = 11
      //! r1      : rank = 11 / pn = 2. 11 - (rank * pn) = 1
      //!           local_ids_begin + 1 = 5 -> put back 5 into queue 2

      std::mutex _notification_guard;

      fhg::util::threadsafe_queue<notification_t> _write_ids;
      notification_t next_write_id();

      notification_id_t _notification_ids_per_node;
      notification_id_t pong_ids_offset() const;
      notification_id_t ping_ids_count() const;
      notification_id_t total_number_of_notifications() const;

      rank_t sending_rank (notification_id_t) const;
      notification_id_t ids_begin (rank_t) const;
      notification_id_t notification_id_offset (notification_id_t) const;
      bool is_pong (notification_id_t) const;
      notification_id_t corresponding_local_ping_id
        (notification_id_t pong_id) const;
      notification_id_t corresponding_local_pong_id
        (notification_id_t ping_id) const;

      std::unordered_map<rank_t, fhg::util::threadsafe_queue<notification_id_t>>
        _ping_ids;
      notification_id_t next_ping_id (rank_t);

      std::condition_variable _notification_received;
      std::map<notification_t, std::size_t> _outstanding_notifications;

      std::atomic<bool> _notification_check_interrupted;
      std::unique_ptr<::boost::strict_scoped_thread<>> _notification_check;
      fhg::util::finally_t<std::function<void()>> _interrupt_notification_check;
      void notification_check();
    };
  }
}
