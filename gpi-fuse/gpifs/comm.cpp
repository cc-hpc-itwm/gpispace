#include <errno.h>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include <gpi-space/pc/type/flags.hpp>
#include <gpi-space/config/parser.hpp>
#include <gpi-space/pc/client/api.hpp>

#include <gpifs/comm.hpp>
#include <gpifs/log.hpp>


static gpi::pc::client::api_t & gpi_api ()
{
  static gpi::pc::client::api_t a;
  return a;
}

#define GPIFS_SHM_SIZE (128 * 1024)

static gpi::pc::type::info::descriptor_t gpi_info;
static void *shm_ptr = 0;
static const size_t shm_size = GPIFS_SHM_SIZE;
static gpi::pc::type::segment_id_t shm_id = 0;
static gpi::pc::type::handle_t     shm_hdl = 0;
static gpi::pc::type::handle_t     scr_hdl = 0;

namespace gpifs
{
  namespace comm
  {
    int comm::init ()
    {
      LOG ("comm: init");

      int res (0);

      namespace fs = boost::filesystem;
      gpi_space::parser::config_parser_t cfg_parser;
      {
        fs::path config_file
          (std::string(getenv("HOME")) + "/.sdpa/configs/sdpa.rc");

        try
        {
          gpi_space::parser::parse (config_file.string(), boost::ref(cfg_parser));
        }
        catch (std::exception const & ex)
        {
          LOG("could not parse config file " << config_file << ": " << ex.what());
          return -EIO;
        }
      }

      fs::path socket_path (cfg_parser.get("gpi.socket_path", "/var/tmp"));
      socket_path /=
        ("S-gpi-space." + boost::lexical_cast<std::string>(getuid()) + ".0");

      gpi_api().path (socket_path.string());

      try
      {
        gpi_api().start ();
      }
      catch (std::exception const & ex)
      {
        LOG( "could not connect to gpi-space at "
           << socket_path
           << ": "
           << ex.what ()
           );
        return -EIO;
      }

      gpi_info = gpi_api().collect_info();

      // register segment

      shm_id = gpi_api().register_segment ( "gpifs-"+boost::lexical_cast<std::string>(getpid ())
                                          , shm_size
                                          , gpi::pc::F_EXCLUSIVE
                                          | gpi::pc::F_FORCE_UNLINK
                                          );

      shm_hdl = gpi_api().alloc ( shm_id
                                , shm_size
                                , "gpifs-transfer"
                                , gpi::pc::F_EXCLUSIVE
                                );
      shm_ptr = gpi_api ().ptr (shm_hdl);

      return res;
    }
    int comm::finalize ()
    {
      LOG ("comm: finalize");

      int res (0);

      gpi_api().stop ();

      return res;
    }

    int comm::free (const alloc::id_t & id) const
    {
      LOG ("comm: free " << id);

      int res (0);

      try
      {
        gpi_api().free (id);
      }
      catch (std::exception const & ex)
      {
        res = -ENOENT;
      }

      return res;
    }
    int comm::alloc (const alloc::descr & descr) const
    {
      LOG ("comm: alloc " << descr);


      try
      {
        gpi_api().alloc ( descr.segment()
                        , descr.size()
                        , descr.name()
                        , gpi::pc::F_PERSISTENT
                        | (descr.global() ? gpi::pc::F_GLOBAL : 0)
                        );
      }
      catch (std::exception const & ex)
      {
        // TODO
        return -ENOMEM;
      }

      return 0;
    }

    namespace detail
    {
      int
      get_handle_info ( const alloc::id_t id
                      , gpi::pc::type::handle::descriptor_t & hdl_info
                      )
      {
        bool found (false);
        {
          gpi::pc::type::handle::list_t handles (gpi_api().list_allocations ());
          for ( gpi::pc::type::handle::list_t::const_iterator it(handles.begin())
              ; it != handles.end() && !found
              ; ++it
              )
          {
            if (it->id == id)
            {
              found = true;
              hdl_info = *it;
            }
          }
        }

        if (!found)
          return -ENOENT;

        // global gpi handles are a bit different
        if (hdl_info.segment == gpifs::segment::GPI
           && (hdl_info.flags & gpi::pc::F_GLOBAL)
           )
        {
          hdl_info.size *= gpi_info.nodes;
        }

        return 0;
      }
    }

    std::size_t comm::read ( const alloc::id_t & id
                           , char * buf
                           , std::size_t size
                           , std::size_t offset
                           ) const
    {
      LOG( "comm: read from handle "
         << id
         << ":"
         << offset
         << " "
         << size
         << " bytes"
         );

      gpi::pc::type::handle::descriptor_t hdl_info;
      {
        int rc = detail::get_handle_info (id, hdl_info);
        if (rc < 0)
        {
          return (size_t)(rc);
        }
      }

      if (hdl_info.segment != segment::GPI)
        return (size_t)(-EINVAL);

      size = std::min (size, hdl_info.size - offset);

      size_t remaining(size);
      while (remaining)
      {
        size_t chunk (std::min (remaining, shm_size));

        try
        {
          gpi_api().wait
            (gpi_api().memcpy ( gpi::pc::type::memory_location_t (shm_hdl, 0)
                              , gpi::pc::type::memory_location_t (id, offset)
                              , chunk
                              , 0
                              )
            );

          memcpy (buf, shm_ptr, chunk);

          buf += chunk;
          remaining -= chunk;
          offset += chunk;
        }
        catch (std::exception const & ex)
        {
          LOG("read: failed: " << ex.what());
          return (size_t)(-EIO);
        }
      }

      return size;
    }

    std::size_t comm::write ( const alloc::id_t & id
                            , const char * buf
                            , std::size_t size
                            , std::size_t offset
                            )
    {
      LOG ( "comm: write to handle " << id << ":" << offset
          << " " << size << " bytes"
          );

      gpi::pc::type::handle::descriptor_t hdl_info;
      {
        int rc = detail::get_handle_info (id, hdl_info);
        if (rc < 0)
        {
          return (size_t)(rc);
        }
      }

      if (hdl_info.segment != gpifs::segment::GPI)
        return (size_t)(-EINVAL);

      size = std::min (size, hdl_info.size - offset);
      size_t remaining(size);
      while (remaining)
      {
        size_t chunk (std::min (remaining, shm_size));

        memcpy (shm_ptr, buf, chunk);

        try
        {
          gpi_api().wait
            (gpi_api().memcpy ( gpi::pc::type::memory_location_t (id, offset)
                              , gpi::pc::type::memory_location_t (shm_hdl, 0)
                              , chunk
                              , 0
                              )
            );

          buf += chunk;
          remaining -= chunk;
          offset += chunk;
        }
        catch (std::exception const & ex)
        {
          LOG("write: failed: " << ex.what());
          return (size_t)(-EIO);
        }
      }

      return size;
    }

    int comm::list_segments (segment::id_list_t * list) const
    {
      LOG ("comm: list_segments");

      int res (0);

      list->clear();
      list->push_back(segment::GPI);

      // try
      // {
      //   gpi::pc::type::segment::list_t
      //     segments (gpi_api().list_segments());


      //   BOOST_FOREACH (gpi::pc::type::segment::list_t::value_type const & seg, segments)
      //   {
      //     if (seg.id == segment::GPI)
      //       }
      // }
      // catch (std::exception const & ex)
      // {
      //   return -EIO;
      // }

      return res;
    }

    namespace detail
    {
      static alloc::alloc
      make_alloc(gpi::pc::type::handle::descriptor_t const & hdl)
      {
        gpifs::alloc::id_t id (hdl.id);
        gpifs::alloc::name_t name (hdl.name);
        gpifs::segment::id_t seg_id (hdl.segment);
        gpifs::alloc::size_t size (hdl.size);
        time_t ctime (hdl.ts.modified);

        // global gpi handles are a bit different
        if (  seg_id == gpifs::segment::GPI
           && (hdl.flags & gpi::pc::F_GLOBAL)
           )
        {
          size *= gpi_info.nodes;
        }

        return alloc::alloc ( id
                            , ctime
                            , seg_id
                            , size
                            , name
                            , (hdl.flags & gpi::pc::F_GLOBAL) != 0
                            );
      }
    }

    int comm::list_allocs (const segment::id_t & id, alloc::list_t * list) const
    {
      LOG ("comm: list_allocs for segment " << segment::string (id));

      int res (0);

      try
      {
        gpi::pc::type::handle::list_t
          handles (gpi_api().list_allocations(id));

        list->clear();

        BOOST_FOREACH (gpi::pc::type::handle::list_t::value_type const & hdl, handles)
        {
          list->push_back(detail::make_alloc(hdl));
        }
      }
      catch (std::exception const & ex)
      {
        return -EIO;
      }

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
