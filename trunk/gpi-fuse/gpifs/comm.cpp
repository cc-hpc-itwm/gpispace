#ifdef COMM_TEST
#include <ctime>
#endif

#include <gpi-space/pc/client/api.hpp>

#include <gpifs/comm.hpp>
#include <gpifs/log.hpp>

static gpi::pc::client::api_t & gpi_api ()
{
  static gpi::pc::client::api_t a;
  return a;
}

namespace gpifs
{
  namespace comm
  {
    int comm::init ()
    {
      LOG ("comm: init");

      int res (0);

      return res;
    }
    int comm::finalize ()
    {
      LOG ("comm: finalize");

      int res (0);

      return res;
    }

    int comm::free (const alloc::id_t & id) const
    {
      LOG ("comm: free " << id);

      int res (0);

      return res;
    }
    int comm::alloc (const alloc::descr & descr) const
    {
      LOG ("comm: alloc " << descr)

        int res (0);

      return res;
    }

    std::size_t comm::read ( const alloc::id_t & id
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
    std::size_t comm::write ( const alloc::id_t & id
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

    int comm::list_segments (segment::id_list_t * list) const
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
    int comm::list_allocs (const segment::id_t & id, alloc::list_t * list) const
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
    void comm::list_segments_test (segment::id_list_t * list) const
    {
      list->push_back (segment::GLOBAL);
      list->push_back (segment::LOCAL);
      list->push_back (4);
      list->push_back (9);
      list->push_back (11);
    }

    void comm::list_allocs_test ( const segment::id_t & id
                          , alloc::list_t * list
                          ) const
    {
      const time_t minute (60);
      const time_t hour (60 * minute);
      const time_t now (time (NULL));

      switch (id)
      {
      case segment::GLOBAL:
        list->push_back (alloc::alloc (0x1006000000000001, now - 10 * minute, segment::GLOBAL, (1 << 30), "vel"));
        list->push_back (alloc::alloc (0x10a0000000000001, now - 20 * minute, segment::GLOBAL, (1 << 20), "trace"));
        list->push_back (alloc::alloc (0x1000000000000002, now - 1 * hour, segment::GLOBAL, (1 << 30), "vol"));
        break;
      case segment::LOCAL:
        list->push_back (alloc::alloc (0x2000000000000004, now - 5 * minute, segment::LOCAL, (23 << 10), "tmp"));
        list->push_back (alloc::alloc (0x2000000000000007, now - 5 * minute, segment::LOCAL, (500 << 20), "scratch"));
        list->push_back (alloc::alloc (0x2000000000000009, now - 12 * minute, segment::LOCAL, (100 << 20), "conn1"));
        list->push_back (alloc::alloc (0x200000000000000b, now - 13 * minute, segment::LOCAL, (100 << 20), "conn2"));
        list->push_back (alloc::alloc (0x2000000000000011, now - 4 * hour, segment::LOCAL, (500 << 20), "out"));
        break;
      case 4:
        list->push_back (alloc::alloc (0x300000000000012d, now  - 5 * hour, 4, (110 << 20), "in"));
        list->push_back (alloc::alloc (0x300000000000012e, now - 5 * hour, 4, (110 << 20), "out"));
        break;
      case 9:
        break;
      case 11:
        list->push_back (alloc::alloc (0x300000000000044d, now - 1 * minute, 11, (101), "a"));
        list->push_back (alloc::alloc (0x300000000000044e, now - 1 * minute, 11, (102), "b"));
        list->push_back (alloc::alloc (0x300000000000045f, now - 1 * minute, 11, (119), "b"));
        break;
      default:
        break;
      }
    }
#endif // COMM_TEST
  } // namespace comm
} // namespace gpifs
