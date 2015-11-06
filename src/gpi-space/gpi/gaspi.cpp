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
    static_assert
      ( std::is_same<notification_t, gaspi_notification_t>::value
      , "HACK: don't expose GASPI.h to users, but we still need the types"
      );
    static_assert
      ( std::is_same<rank_t, gaspi_rank_t>::value
      , "HACK: don't expose GASPI.h to users, but we still need the types"
      );
    static_assert
      ( std::is_same<notification_id_t, gaspi_notification_id_t>::value
      , "HACK: don't expose GASPI.h to users, but we still need the types"
      );

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

    namespace
    {
      struct time_left
      {
        template<typename Duration>
          time_left (Duration timeout)
            : _end (std::chrono::steady_clock::now() + timeout)
        {}
        gaspi_timeout_t operator()() const
        {
          auto const left
            ( std::chrono::duration_cast<std::chrono::milliseconds>
                (_end - std::chrono::steady_clock::now()).count()
            );

          return left < 0 ? GASPI_TEST : left;
        }

      private:
        std::chrono::steady_clock::time_point const _end;
      };
    }

    gaspi_t::gaspi_t ( fhg::log::Logger& logger
                     , const unsigned long long memory_size
                     , const unsigned short port
                     , const std::chrono::seconds& timeout
                     , unsigned short communication_port
                     )
      : _logger (logger)
      , m_mem_size (memory_size)
      , m_dma (nullptr)
      , m_replacement_gpi_segment (0)
      , _next_write_id (0)
    {
      time_left const time_left (timeout);

      gaspi_config_t config;
      FAIL_ON_NON_ZERO (gaspi_config_get, &config);
      config.sn_port = port;
      FAIL_ON_NON_ZERO (gaspi_config_set, config);

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

      FAIL_ON_NON_ZERO (gaspi_proc_init, time_left());
      FAIL_ON_NON_ZERO ( gaspi_segment_create
                       , m_replacement_gpi_segment
                       , m_mem_size
                       , GASPI_GROUP_ALL
                       , time_left()
                       , GASPI_MEM_UNINITIALIZED
                       );
      FAIL_ON_NON_ZERO ( gaspi_segment_ptr
                       , m_replacement_gpi_segment
                       , &m_dma
                       );

      FAIL_ON_NON_ZERO (gaspi_transfer_size_max, &_max_transfer_size);

      gaspi_number_t available_notifications;
      FAIL_ON_NON_ZERO (gaspi_notification_num, &available_notifications);

      //! \todo reasonable maximum
      constexpr gaspi_number_t const maximum_notifications_per_rank (16);

      _notification_ids_per_node
        = std::min ( available_notifications
                   / gaspi_number_t (number_of_nodes())
                   , maximum_notifications_per_rank
                   );
      if (_notification_ids_per_node < 2)
      {
        throw std::runtime_error
          ( "need at least two notification ids per rank ("
          + std::to_string (available_notifications)
          + " notification ids available in total, but "
          + std::to_string (number_of_nodes()) + " ranks)"
          );
      }

      _maximum_notification_id
        = _notification_ids_per_node * number_of_nodes();

      _local_ids_begin = rank() * _notification_ids_per_node;
      _pong_ids_offset = _notification_ids_per_node / 2;

      {
        auto thread
          ( fhg::util::cxx14::make_unique<decltype (_notification_check)::element_type>
              (&gaspi_t::notification_check, this)
          );
        std::swap (_notification_check, thread);
      }

      {
        std::vector<gaspi_notification_id_t> ping_ids (_pong_ids_offset);
        std::iota (ping_ids.begin(), ping_ids.end(), _local_ids_begin);
        for (gaspi_rank_t rank (0); rank < number_of_nodes(); ++rank)
        {
          _ping_ids[rank].put_many (ping_ids.begin(), ping_ids.end());
        }
      }

      struct hostname_and_port_t
      {
        char hostname[HOST_NAME_MAX + 1]; // + \0
        unsigned short port;
      };

      const gaspi_segment_id_t exchange_hostname_and_port_segment {1};

      FAIL_ON_NON_ZERO ( gaspi_segment_create
                       , exchange_hostname_and_port_segment
                       , 2 * sizeof (hostname_and_port_t)
                       , GASPI_GROUP_ALL
                       , time_left()
                       , GASPI_MEM_UNINITIALIZED
                       );
      void* exchange_hostname_and_port_data_raw;
      FAIL_ON_NON_ZERO ( gaspi_segment_ptr
                       , exchange_hostname_and_port_segment
                       , &exchange_hostname_and_port_data_raw
                       );

      hostname_and_port_t* exchange_hostname_and_port_data_send
        (static_cast<hostname_and_port_t*> (exchange_hostname_and_port_data_raw));
      hostname_and_port_t* exchange_hostname_and_port_data_receive
        (exchange_hostname_and_port_data_send + 1);

      strncpy ( exchange_hostname_and_port_data_send->hostname
              , fhg::util::hostname().c_str()
              , sizeof (exchange_hostname_and_port_data_send->hostname)
              );
      exchange_hostname_and_port_data_send->port = communication_port;

      FAIL_ON_NON_ZERO (gaspi_barrier, GASPI_GROUP_ALL, time_left());

      m_rank_to_hostname.resize (number_of_nodes());
      _communication_port_by_rank.resize (number_of_nodes());

      m_rank_to_hostname[rank()] =
        exchange_hostname_and_port_data_send->hostname;
      _communication_port_by_rank[rank()] =
        exchange_hostname_and_port_data_send->port;

      for ( gaspi_rank_t r ((rank() + 1) % number_of_nodes())
          ; r != rank()
          ; r = (r + 1) % number_of_nodes()
          )
      {
        memset ( exchange_hostname_and_port_data_receive
               , 0
               , sizeof (hostname_and_port_t)
               );

        FAIL_ON_NON_ZERO ( gaspi_read
                         , exchange_hostname_and_port_segment
                         , sizeof (hostname_and_port_t)
                         , r
                         , exchange_hostname_and_port_segment
                         , 0
                         , sizeof (hostname_and_port_t)
                         , 0
                         , time_left()
                         );
        FAIL_ON_NON_ZERO (gaspi_wait, 0, time_left());

        m_rank_to_hostname[r] =
          exchange_hostname_and_port_data_receive->hostname;
        _communication_port_by_rank[r] =
          exchange_hostname_and_port_data_receive->port;
      }

      FAIL_ON_NON_ZERO (gaspi_barrier, GASPI_GROUP_ALL, time_left());
      FAIL_ON_NON_ZERO (gaspi_segment_delete, exchange_hostname_and_port_segment);
    }

    gaspi_t::~gaspi_t()
    {
      _notification_check.reset();

      FAIL_ON_NON_ZERO (gaspi_proc_term, GASPI_BLOCK);
    }

    gpi::size_t gaspi_t::number_of_queues() const
    {
      gaspi_number_t queue_num;
      FAIL_ON_NON_ZERO (gaspi_queue_num, &queue_num);
      return queue_num;
    }

    gpi::size_t gaspi_t::number_of_nodes() const
    {
      gaspi_rank_t num_ranks;
      FAIL_ON_NON_ZERO (gaspi_proc_num, &num_ranks);
      return num_ranks;
    }

    gpi::size_t gaspi_t::memory_size() const
    {
      return m_mem_size;
    }

    gpi::rank_t gaspi_t::rank() const
    {
      gaspi_rank_t rank;
      FAIL_ON_NON_ZERO (gaspi_proc_rank, &rank);
      return rank;
    }

    std::string const& gaspi_t::hostname_of_rank (const gpi::rank_t r) const
    {
      return m_rank_to_hostname[r];
    }

    unsigned short gaspi_t::communication_port_of_rank (gpi::rank_t rank) const
    {
      return _communication_port_by_rank[rank];
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
      //! \todo round-robin queues again!
      queue_desc_t queue (0);

      gaspi_number_t queue_size_max;
      FAIL_ON_NON_ZERO (gaspi_queue_size_max, &queue_size_max);

      gaspi_number_t queue_size;
      FAIL_ON_NON_ZERO (gaspi_queue_size, queue, &queue_size);

      if (queue_size >= queue_size_max)
      {
        wait_dma (queue);
      }

      FAIL_ON_NON_ZERO ( std::forward<Fun> (function)
                       , std::forward<Args> (arguments)...
                       , queue
                       , GASPI_BLOCK
                       );

      return queue;
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
                                , m_replacement_gpi_segment
                                , l_off
                                , from_node
                                , m_replacement_gpi_segment
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
            wait_dma (queue);
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
      std::unordered_set<queue_desc_t> queues;

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

        queues.emplace
          ( queued_operation<2> ( gaspi_write_notify
                                , m_replacement_gpi_segment
                                , l_off
                                , to_node
                                , m_replacement_gpi_segment
                                , r_off
                                , to_transfer
                                , next_ping_id (to_node)
                                , write_id
                                )
          );

        remaining -= to_transfer;
        l_off     += to_transfer;
        r_off     += to_transfer;
      }

      return {queues, write_id};
    }

    void gaspi_t::wait_buffer_reusable
      (std::list<write_dma_info> const& infos)
    {
      //! \todo only check for specific buffers? possible with gaspi?!
      std::unordered_set<queue_desc_t> waited_queues;
      for (write_dma_info const& info : infos)
      {
        for (auto const& queue : info.queues)
        {
          if (waited_queues.emplace (queue).second)
          {
            wait_dma (queue);
          }
        }
      }
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
        }
      }
    }

    void gaspi_t::notification_check()
    {
      for (;;)
      {
        boost::this_thread::interruption_point();

        gaspi_notification_id_t notification_id;
        gaspi_return_t const waitsome_result
          ( gaspi_notify_waitsome
              ( m_replacement_gpi_segment
              , 0
              , _maximum_notification_id
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
                         , m_replacement_gpi_segment
                         , notification_id
                         , &write_id
                         );

        gaspi_rank_t const sending_rank
          (notification_id / _notification_ids_per_node);
        gaspi_notification_id_t notification_id_offset
          (notification_id - (sending_rank * _notification_ids_per_node));
        bool const is_pong (notification_id_offset >= _pong_ids_offset);

        if (is_pong)
        {
          {
            std::unique_lock<std::mutex> const _ (_notification_guard);
            --_outstanding_notifications.at (write_id);
          }

          _ping_ids[sending_rank].put ( notification_id_offset
                                      - _pong_ids_offset
                                      + _local_ids_begin
                                      );
          _notification_received.notify_all();
        }
        else
        {
          queued_operation<1> ( gaspi_notify
                              , m_replacement_gpi_segment
                              , sending_rank
                              , notification_id_offset
                              + _pong_ids_offset
                              + _local_ids_begin
                              , write_id
                              );
        }
      }
    }

    notification_t gaspi_t::next_write_id()
    {
      std::unique_lock<std::mutex> const _ (_notification_guard);
      notification_t write_id (++_next_write_id);
      if (!write_id)
      {
        write_id = ++_next_write_id;
      }
      return write_id;
    }
    notification_id_t gaspi_t::next_ping_id (rank_t rank)
    {
      return _ping_ids[rank].get();
    }

    void gaspi_t::wait_dma (const queue_desc_t queue)
    {
      FAIL_ON_NON_ZERO (gaspi_wait, queue, GASPI_BLOCK);
    }

#undef FAIL_ON_NON_ZERO
  }
}
