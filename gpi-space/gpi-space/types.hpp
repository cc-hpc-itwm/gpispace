#ifndef GPI_SPACE_TYPES_HPP
#define GPI_SPACE_TYPES_HPP 1

#include <inttypes.h>

#include <boost/dynamic_bitset.hpp>

namespace gpi
{
  typedef uint64_t offset_t;
  typedef uint64_t rank_t;
  typedef uint64_t timeout_t;
  typedef uint64_t queue_desc_t; // queue descriptor
  typedef uint64_t size_t;
  typedef float    version_t;
  typedef unsigned short port_t;
  typedef int      network_type_t;
  typedef boost::dynamic_bitset<> error_vector_t;
}

#endif
