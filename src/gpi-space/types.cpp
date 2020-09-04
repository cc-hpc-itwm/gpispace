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

#include <gpi-space/types.hpp>

#include <GASPI.h>

namespace gpi
{
  //! \note don't expose GASPI.h to users, but we still correct types
  static_assert
    (std::is_same<offset_t, gaspi_offset_t>::value, "offset_t");
  static_assert
    (std::is_same<rank_t, gaspi_rank_t>::value, "rankt_");
  static_assert
    (std::is_same<queue_desc_t, gaspi_queue_id_t>::value, "queue_id_t");
  static_assert
    (std::is_same<size_t, gaspi_size_t>::value, "size_t");
  static_assert
    (std::is_same<notification_t, gaspi_notification_t>::value, "notification_t");
  static_assert
    (std::is_same<notification_id_t, gaspi_notification_id_t>::value, "notification_id_t");
  static_assert
    (std::is_same<timeout_t, gaspi_timeout_t>::value, "timeout_t");
  static_assert
    (std::is_same<segment_id_t, gaspi_segment_id_t>::value, "segment_id_t");
  static_assert
    (std::is_same<netdev_id_t, gaspi_int>::value, "netdev_id_t");
}
