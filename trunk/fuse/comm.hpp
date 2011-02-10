// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_COMM_HPP
#define FUSE_COMM_HPP 1

#include <segment.hpp>
#include <alloc.hpp>

#ifdef COMM_TEST
#include <ctime>
#endif

#include <log.hpp>

namespace gpifs
{
  namespace comm
  {
    class comm
    {
    public:
      int init ()
      {
        LOG ("comm: init");

        int res (0);

        return res;
      }
      int finalize ()
      {
        LOG ("comm: finalize");

        int res (0);

        return res;
      }

      int free (const alloc::id_t & id) const
      {
        LOG ("comm: free " << id);

        int res (0);

        return res;
      }
      int alloc (const alloc::descr & descr) const
      {
        LOG ("comm: alloc " << descr)

        int res (0);

        return res;
      }

      std::size_t read ( const alloc::id_t & id
                       , char * buf
                       , std::size_t size
                       , const std::size_t offset
                       ) const
      {
        LOG ( "comm: read from handle " << id << ":" << offset
            << " " << size << " bytes"
            );

        return 0;
      }
      std::size_t write ( const alloc::id_t & id
                        , const char * buf
                        , std::size_t size
                        , const std::size_t offset
                        )
      {
        LOG ( "comm: write to handle " << id << ":" << offset
            << " " << size << " bytes"
            );

        return size;
      }

      int list_segments (segment::id_list_t * list) const
      {
        LOG ("comm: list_segments");

        int res (0);

        list->clear();

#ifdef COMM_TEST
        list_segments_test (list);
#else // ! ifdef COMM_TEST
#endif // ifdef COMM_TEST

        return res;
      }
      int list_allocs (const segment::id_t & id, alloc::list_t * list) const
      {
        LOG ("comm: list_allocs for segment " << segment::string (id));

        int res (0);

        list->clear();

#ifdef COMM_TEST
        list_allocs_test (id, list);
#else // ! ifdef COMM_TEST
#endif // ifdef COMM_TEST

        return res;
      }

      // ******************************************************************* //

#ifdef COMM_TEST
      void list_segments_test (segment::id_list_t * list) const
      {
        list->push_back (0);
        list->push_back (1);
        list->push_back (3);
        list->push_back (9);
        list->push_back (11);
      }

      void list_allocs_test ( const segment::id_t & id
                            , alloc::list_t * list
                            ) const
      {
        const time_t minute (60);
        const time_t hour (60 * minute);
        const time_t now (time (NULL));

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
            break;
          }
      }
#endif // ifdef COMM_TEST
    };
  } // namespace comm
} // namespace gpu_fuse

#endif
