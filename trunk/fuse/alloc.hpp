// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_ALLOC_HPP
#define FUSE_ALLOC_HPP

#include <boost/unordered_set.hpp>

#include <list>

#include <id.hpp>

namespace gpi_fuse
{
  namespace alloc
  {
    typedef id::id_t id_t;
    typedef std::list<id_t> id_list_t;
    typedef boost::unordered_set<alloc::id_t> set_t;
  } // namespace alloc
} // namespace gpi_fuse

#endif
