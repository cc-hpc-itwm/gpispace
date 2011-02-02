// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_COMM_HPP
#define FUSE_COMM_HPP 1

#include <stdexcept>

#include <segment.hpp>
#include <alloc.hpp>

namespace gpi_fuse
{
  namespace comm
  {
    class comm
    {
    public:
      void init () {}
      void finalize () {}

#ifdef COMM_TEST
      void list_segments (segment::id_list_t * list) const
      {
        list->clear();

        list->push_back (0);
        list->push_back (1);
        list->push_back (3);
        list->push_back (9);
      }

      void list_allocs ( const segment::id_t & id
                       , alloc::id_list_t * list
                       ) const
      {
        list->clear();

        switch (id)
          {
          case 0:
            list->push_back (0);
            list->push_back (1);
            list->push_back (2);
            break;
          case 1:
            list->push_back (4);
            list->push_back (7);
            list->push_back (9);
            break;
          case 3:
            list->push_back (301);
            list->push_back (302);
            break;
          case 9:
            break;
          default:
            throw std::runtime_error ("segment unknown");
          }
      }
#else
      void list_segments (segment::id_list_t *) const
      {
      }
      void list_allocs ( const segment::id_t &
                       , alloc::id_list_t *
                       ) const
      {
      }
#endif
    };
  } // namespace comm
} // namespace gpu_fuse

#endif
