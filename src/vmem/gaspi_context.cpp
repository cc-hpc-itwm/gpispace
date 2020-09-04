// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <gpi-space/gpi/gaspi.hpp>

#include <gpi-space/exception.hpp>

#include <util-generic/hostname.hpp>

#include <GASPI.h>
#include <GASPI_Ext.h>

#include <algorithm>
#include <limits>
#include <unordered_set>

namespace fhg
{
  namespace vmem
  {
    namespace
    {
      void throw_gaspi_error ( std::string const& function_name
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

    gpi::timeout_t gaspi_timeout::operator()() const
    {
      auto const left
        ( std::chrono::duration_cast<std::chrono::milliseconds>
            (_end - std::chrono::steady_clock::now()).count()
        );

      return left < 0 ? GASPI_TEST : left;
    }

    gaspi_context::gaspi_context ( gaspi_timeout& time_left
                                 , unsigned short gaspi_sn_port
                                 , unsigned short local_communication_port
                                 , netdev_id netdev_id
                                 )
    {
      gaspi_config_t config;
      FAIL_ON_NON_ZERO (gaspi_config_get, &config);
      config.sn_port = gaspi_sn_port;
      config.netdev_id = netdev_id.value;
      FAIL_ON_NON_ZERO (gaspi_config_set, config);

      FAIL_ON_NON_ZERO (gaspi_proc_init, time_left());

      {
        gaspi_number_t segment_max;
        FAIL_ON_NON_ZERO (gaspi_segment_max, &segment_max);

        gpi::segment_id_t id (0);
        std::generate_n ( std::inserter (_segment_ids, _segment_ids.end())
                        , segment_max
                        , [&id] { return id++; }
                        );
      }

      struct hostname_and_port_t
      {
        char hostname[HOST_NAME_MAX + 1]; // + \0
        unsigned short port;
      };

      reserved_segment_id const exchange_hostname_and_port_segment (*this);

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
              , HOST_NAME_MAX
              );
      exchange_hostname_and_port_data_send->hostname[HOST_NAME_MAX] = '\0';
      exchange_hostname_and_port_data_send->port = local_communication_port;

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

    gaspi_context::~gaspi_context()
    {
      FAIL_ON_NON_ZERO (gaspi_proc_term, GASPI_BLOCK);
    }

    gpi::size_t gaspi_context::number_of_queues() const
    {
      gaspi_number_t queue_num;
      FAIL_ON_NON_ZERO (gaspi_queue_num, &queue_num);
      return queue_num;
    }

    gpi::size_t gaspi_context::number_of_nodes() const
    {
      gaspi_rank_t num_ranks;
      FAIL_ON_NON_ZERO (gaspi_proc_num, &num_ranks);
      return num_ranks;
    }

    gpi::rank_t gaspi_context::rank() const
    {
      gaspi_rank_t rank;
      FAIL_ON_NON_ZERO (gaspi_proc_rank, &rank);
      return rank;
    }

    std::string const& gaspi_context::hostname_of_rank (gpi::rank_t r) const
    {
      return m_rank_to_hostname[r];
    }

    unsigned short gaspi_context::communication_port_of_rank (gpi::rank_t rank) const
    {
      return _communication_port_by_rank[rank];
    }

    gaspi_context::reserved_segment_id::reserved_segment_id (gaspi_context& context)
      : _context (context)
    {
      std::lock_guard<std::mutex> const _ (_context._segment_id_guard);
      if (_context._segment_ids.empty())
      {
        throw std::runtime_error ("no segment id available");
      }
      _id = *_context._segment_ids.begin();
      _context._segment_ids.erase (_context._segment_ids.begin());
    }

    gaspi_context::reserved_segment_id::~reserved_segment_id()
    {
      std::lock_guard<std::mutex> const _ (_context._segment_id_guard);
      _context._segment_ids.emplace (_id);
    }

#undef FAIL_ON_NON_ZERO
  }
}
