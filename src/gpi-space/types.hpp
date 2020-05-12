#pragma once

#include <inttypes.h>

#include <boost/dynamic_bitset.hpp>

namespace gpi
{
  typedef uint64_t offset_t;
  using rank_t = unsigned short;
  using queue_desc_t = unsigned char;
  typedef uint64_t size_t;
  typedef float    version_t;
  typedef unsigned short port_t;
  typedef boost::dynamic_bitset<> error_vector_t;

  using notification_t = unsigned int;
  using notification_id_t = unsigned short;

  using timeout_t = unsigned long;
  using segment_id_t = unsigned char;

  using netdev_id_t = int;
}
