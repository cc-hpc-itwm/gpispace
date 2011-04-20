// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_COMM_HPP
#define FUSE_COMM_HPP 1

#include <boost/lexical_cast.hpp>

#include <gpifs/segment.hpp>
#include <gpifs/alloc.hpp>

namespace gpifs
{
  namespace comm
  {
    class comm
    {
    public:
      int init ();
      int finalize ();

      int free (const alloc::id_t & id) const;
      int alloc (const alloc::descr & descr) const;
      std::size_t read ( const alloc::id_t & id
                       , char * buf
                       , std::size_t size
                       , std::size_t offset
                       ) const;
      std::size_t write ( const alloc::id_t & id
                        , const char * buf
                        , std::size_t size
                        , std::size_t offset
                        );

      int list_segments (segment::id_list_t * list) const;
      int list_allocs (const segment::id_t & id, alloc::list_t * list) const;
    };
  } // namespace comm
} // namespace gpu_fuse

#endif
