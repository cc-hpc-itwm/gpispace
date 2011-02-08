// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_COMM_HPP
#define FUSE_COMM_HPP 1

#include <stdexcept>

#include <segment.hpp>
#include <alloc.hpp>

#ifdef COMM_TEST
#include <ctime>
#endif

namespace gpifs
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
      {
      }
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
            list->push_back (alloc::alloc (0, now - 10 * minute, 0, (1 << 30), "vel"));
            list->push_back (alloc::alloc (1, now - 20 * minute, 0, (1 << 20), "trace"));
            list->push_back (alloc::alloc (2, now - 1 * hour, 0, (1 << 30), "vol"));
            break;
          case 1:
            list->push_back (alloc::alloc (4, now - 5 * minute, 1, (23 << 10), "tmp"));
            list->push_back (alloc::alloc (7, now - 5 * minute, 1, (500 << 20), "scratch"));
            list->push_back (alloc::alloc (9, now - 12 * minute, 1, (100 << 20), "conn1"));
            list->push_back (alloc::alloc (11, now - 13 * minute, 1, (100 << 20), "conn2"));
            list->push_back (alloc::alloc (17, now - 4 * hour, 1, (500 << 20), "out"));
            break;
          case 3:
            list->push_back (alloc::alloc (301, now  - 5 * hour, 3, (110 << 20), "in"));
            list->push_back (alloc::alloc (302, now - 5 * hour, 3, (110 << 20), "out"));
            break;
          case 9:
            break;
          case 11:
            list->push_back (alloc::alloc (1101, now - 1 * minute, 11, (101), "a"));
            list->push_back (alloc::alloc (1102, now - 1 * minute, 11, (102), "b"));
            list->push_back (alloc::alloc (1119, now - 1 * minute, 11, (119), "b"));
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
