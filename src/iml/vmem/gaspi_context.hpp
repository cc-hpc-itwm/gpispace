// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <iml/gaspi/NetdevID.hpp>
#include <iml/vmem/gaspi/types.hpp>

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

namespace fhg
{
  namespace iml
  {
    namespace gaspi = ::iml::gaspi;
    namespace vmem
    {
      struct gaspi_timeout
      {
        template<typename Duration>
          gaspi_timeout (Duration timeout)
            : _end (std::chrono::steady_clock::now() + timeout)
        {}
        gpi::timeout_t operator()() const;

      private:
        std::chrono::steady_clock::time_point const _end;
      };

      class gaspi_context
      {
      public:
        gaspi_context ( gaspi_timeout&
                      , gpi::port_t gaspi_sn
                      , gpi::port_t local_communication
                      , gaspi::NetdevID netdev_id
                      );
        ~gaspi_context();
        gaspi_context (gaspi_context const&) = delete;
        gaspi_context (gaspi_context&&) = delete;
        gaspi_context& operator= (gaspi_context const&) = delete;
        gaspi_context& operator= (gaspi_context&&) = delete;

        gpi::size_t memory_size() const;
        gpi::rank_t rank() const;
        gpi::size_t number_of_nodes() const;

        std::string const& hostname_of_rank (gpi::rank_t) const;
        gpi::port_t communication_port_of_rank (gpi::rank_t) const;

        //! \todo do not expose: used to determine count of memory
        //! management task threads, which is just bad.
        gpi::size_t number_of_queues() const;

        struct reserved_segment_id
        {
          reserved_segment_id (gaspi_context&);
          ~reserved_segment_id();
          reserved_segment_id (reserved_segment_id const&) = delete;
          reserved_segment_id (reserved_segment_id&&) = delete;
          reserved_segment_id& operator= (reserved_segment_id const&) = delete;
          reserved_segment_id& operator= (reserved_segment_id&&) = delete;

          operator gpi::segment_id_t() const { return _id; }

        private:
          gaspi_context& _context;
          gpi::segment_id_t _id;
        };

      private:
        std::vector<std::string> m_rank_to_hostname;
        std::vector<unsigned short> _communication_port_by_rank;

        std::mutex _segment_id_guard;
        std::unordered_set<gpi::segment_id_t> _segment_ids;
      };
    }
  }
}
