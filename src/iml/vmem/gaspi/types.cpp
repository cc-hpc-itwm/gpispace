#include <iml/vmem/gaspi/types.hpp>

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
