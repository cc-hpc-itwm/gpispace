// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_COMM_HPP
#define FUSE_COMM_HPP 1

#include <stdexcept>

#include <segment.hpp>
#include <alloc.hpp>

#ifdef COMM_TEST
#include <ctime>
#endif

namespace gpi_fuse
{
  namespace comm
  {
    class comm
    {
    public:
      void init ()
      {
      }

      void finalize ()
      {
      }

      void free (const alloc::id_t & id) const
      {
      }

#ifndef COMM_TEST
      void list_segments (segment::id_list_t *) const
      {}
      void list_allocs ( const segment::id_t &
                       , alloc::list_t *
                       ) const
      {}
#else // ifndef COMM_TEST
      void list_segments (segment::id_list_t * list) const
      {
        list->clear();

        list->push_back (0);
        list->push_back (1);
        list->push_back (3);
        list->push_back (9);
        list->push_back (11);
      }

      void list_allocs ( const segment::id_t & id
                       , alloc::list_t * list
                       ) const
      {
        const time_t minute (60);
        const time_t hour (60 * minute);
        const time_t now (time (NULL));

        list->clear();

        switch (id)
          {
          case 0:
            list->push_back (alloc::descr_t (0, "vel", (1 << 30), now - 10 * minute, 0));
            list->push_back (alloc::descr_t (1, "trace", (1 << 20), now - 20 * minute, 0));
            list->push_back (alloc::descr_t (2, "vol", (1 << 30), now - 1 * hour, 0));
            break;
          case 1:
            list->push_back (alloc::descr_t (4, "tmp", (23 << 10), now - 5 * minute, 1));
            list->push_back (alloc::descr_t (7, "scratch", (500 << 20), now - 5 * minute, 1));
            list->push_back (alloc::descr_t (9, "conn1", (100 << 20), now - 12 * minute, 1));
            list->push_back (alloc::descr_t (11, "conn2", (100 << 20), now - 13 * minute, 1));
            list->push_back (alloc::descr_t (17, "out", (500 << 20), now - 4 * hour, 1));
            break;
          case 3:
            list->push_back (alloc::descr_t (301, "in", (110 << 20), now  - 5 * hour, 3));
            list->push_back (alloc::descr_t (302, "out", (110 << 20), now - 5 * hour, 3));
            break;
          case 9:
            break;
          case 11:
            list->push_back (alloc::descr_t (1101, "a", 101, now - 1 * minute, 11));
            list->push_back (alloc::descr_t (1102, "b", 102, now - 1 * minute, 11));
            list->push_back (alloc::descr_t (1119, "b", 119, now - 1 * minute, 11));
            break;
          default:
            throw std::runtime_error ("segment unknown");
          }
      }
#endif // ifndef COMM_TEST
    };
  } // namespace comm
} // namespace gpu_fuse

#endif
