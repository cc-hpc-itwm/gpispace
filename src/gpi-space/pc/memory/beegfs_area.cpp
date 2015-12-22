#include <gpi-space/pc/memory/beegfs_area.hpp>

#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <cstring> // strerror

#include <fhglog/LogMacros.hpp>
#include <gpi-space/pc/url.hpp>
#include <util-generic/finally.hpp>
#include <util-generic/hostname.hpp>
#include <util-generic/print_exception.hpp>
#include <fhg/util/boost/optional.hpp>
#include <fhg/util/read_bool.hpp>
#include <util-generic/read_file.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/write_file.hpp>

#include <vmem/segment/beegfs.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/system/system_error.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <gpi-space/pc/type/flags.hpp>

namespace fs = boost::filesystem;

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      namespace detail
      {
        static beegfs_area_t::path_t meta_path (beegfs_area_t::path_t const &p)
        {
          return p / "meta";
        }

        static beegfs_area_t::path_t data_path (beegfs_area_t::path_t const &p)
        {
          return p / "data";
        }

        static beegfs_area_t::path_t version_path (beegfs_area_t::path_t const &p)
        {
          return p / "version";
        }

        static beegfs_area_t::path_t lock_path (beegfs_area_t::path_t const &p)
        {
          return p / "lock";
        }
      }
      namespace
      {
        struct lock_info
        {
          std::string hostname;
          pid_t pid;

          bool operator== (lock_info const& other) const
          {
            return std::tie (hostname, pid)
              == std::tie (other.hostname, other.pid);
          }

          lock_info()
            : hostname (fhg::util::hostname())
            , pid (fhg::util::syscall::getpid())
          {}

          lock_info (boost::filesystem::path const& path)
          {
            std::string const content (fhg::util::read_file (path));
            std::istringstream iss (content);
            boost::archive::text_iarchive ia (iss);
            ia & hostname;
            ia & pid;
          }
          void write (int fd) const
          {
            std::ostringstream oss;
            boost::archive::text_oarchive oa (oss);
            oa & hostname;
            oa & pid;
            std::string const content (oss.str());
            fhg::util::syscall::write (fd, content.data(), content.size());
          }
        };

        struct scoped_file
        {
          scoped_file ( boost::filesystem::path const& path
                      , int flags
                      , int perms
                      )
            : _fd ( fhg::util::syscall::open
                      (path.string().c_str(), flags, perms)
                  )
          {}
          ~scoped_file()
          {
            fhg::util::syscall::close (_fd);
          }
          void ftruncate (std::size_t size)
          {
            fhg::util::syscall::ftruncate (_fd, size);
          }

          int _fd;
        };
      }

      beegfs_area_t::beegfs_area_t ( fhg::log::Logger& logger
                                   , const gpi::pc::type::process_id_t creator
                                   , const beegfs_area_t::path_t & path
                                   , const gpi::pc::type::size_t size        // total
                                   , const gpi::pc::type::flags_t flags
                                   , gpi::pc::global::itopology_t & topology
                                   , handle_generator_t& handle_generator
                                   )
        : area_t ( logger
                 , beegfs_area_t::area_type
                 , creator
                 , path.string ()
                 , size
                 , flags
                 , handle_generator
                 )
        , m_lock_fd (-1)
        , m_path (path)
        , m_version (BEEGFS_AREA_VERSION)
        , m_size (size)
        , m_topology (topology)
      {
        try
        {
          open();
        }
        catch (...)
        {
          std::throw_with_nested
            (std::runtime_error ("opening '" + path.string() + "' failed"));
        }

        if (m_size != size)
        {
          area_t::reinit ();
        }
      }

      beegfs_area_t::~beegfs_area_t ()
      {
        try
        {
          close();
        }
        catch (...)
        {
          LLOG (ERROR, _logger, fhg::util::current_exception_printer());
        }
      }

      void beegfs_area_t::cleanup (beegfs_area_t::path_t const &path)
      {
        fs::remove_all (path);
      }

      std::string beegfs_area_t::version_string() const
      {
        return "BEEGFS segment version " + std::to_string (m_version);
      }

      void beegfs_area_t::open()
      {
        /* steps:

           - check if path exists and whether it's a directory or not

           - if it doesn't exist: create files

           - locate required files: meta, data, version

           - read the version file, check version number

           - open meta file, consistency check
           - open data file, consistency check
           - map memory
        */
        if (fs::exists (m_path) && gpi::flag::is_set ( descriptor ().flags
                                                     , gpi::pc::F_FORCE_UNLINK
                                                     )
           && get_owner()
           )
        {
          beegfs_area_t::cleanup (m_path);
        }

        if (! fs::exists (m_path) && !gpi::flag::is_set ( descriptor ().flags
                                                        , gpi::pc::F_NOCREATE
                                                        )
           && get_owner()
           )
        {
          initialize (m_path, m_size);
        }

        // read version information
        {
          boost::optional<std::string> const found_version
            ( fhg::util::boost::exception_is_none
                (&fhg::util::read_file, detail::version_path (m_path))
            );

          if (!!found_version && found_version.get() != version_string())
          {
            throw std::logic_error
              ( "the file segment was created by a newer version:"
                " found: " + found_version.get()
              + " wanted: " + version_string()
              );
          }
        }

          // lock it
          if (get_owner())
          {
            lock_info const local_lock_info;

            path_t lock_file = detail::lock_path (m_path);

            // check for lock file
            // abort if it exists -> we have it already open?
            try
            {
              m_lock_fd = fhg::util::syscall::open
                ( lock_file.string ().c_str ()
                , O_CREAT + O_RDWR + O_EXCL
                , S_IRUSR + S_IWUSR
                );
            }
            catch (...)
            {
              // still in use?

              // lock was successful, check who owns/owned it and abort
              lock_info const existing_lock_info (lock_file);

              if (local_lock_info == existing_lock_info)
              {
                throw std::logic_error ("segment already open");
              }

              if (existing_lock_info.hostname != local_lock_info.hostname)
              {
                throw std::runtime_error
                  ( "beegfs segment may still be in use by: host="
                  + existing_lock_info.hostname + " pid="
                  + std::to_string (existing_lock_info.pid)
                  + ", if this is wrong, remove " + lock_file.string()
                  );
              }

              m_lock_fd = fhg::util::syscall::open
                ( lock_file.string ().c_str ()
                , O_CREAT + O_RDWR
                , S_IRUSR + S_IWUSR
                );
            }

            if (lockf (m_lock_fd, F_TLOCK, 0) < 0)
            {
              fhg::util::syscall::close (m_lock_fd);
              m_lock_fd = -1;
              throw std::runtime_error ("unable to lock segment");
            }

            local_lock_info.write (m_lock_fd);

            fhg::util::syscall::fdatasync (m_lock_fd);
          }

          // try to open existing file
          path_t data_path = detail::data_path (m_path);
          int fd = fhg::util::syscall::open ( data_path.string ().c_str ()
                                            , O_RDWR
                                            );
          FHG_UTIL_FINALLY ([&] { fhg::util::syscall::close (fd); });

          fhg::vmem::segment::beegfs::check_requirements (fd);

          off_t file_size = fhg::util::syscall::lseek (fd, 0, SEEK_END);
          fhg::util::syscall::lseek (fd, 0, SEEK_SET);

          if (m_size)
          {
            if (m_size != (gpi::pc::type::size_t)file_size)
            {
              LLOG ( WARN
                   , _logger
                   , "segment has size: " << file_size
                   << " but tried to open it with size: " << m_size
                   << ", adjusting..."
                   );
              m_size = file_size;
            }
          }
          else
          {
            // used in non-create mode
            m_size = file_size;
          }

          descriptor ().local_size = m_size;

          //! \todo better constant (number of threads? configurable?)
          for (int i (0); i < 20; ++i)
          {
            _fds.put (fhg::util::syscall::open (data_path.string().c_str(), O_RDWR));
          }

        LLOG ( TRACE
             , _logger
             , "BEEGFS memory created:"
             << " path: " << m_path
             << " size: " << m_size
             );
      }

      void beegfs_area_t::close()
      {
        // only cleanup when we actually opened...
        {
          /*
            steps:

            - check that no ongoing memory transfers are accessing this area
            - unmap memory
            - depending on the flags, remove the directory again
          */

          if (gpi::flag::is_set ( descriptor ().flags
                                , gpi::pc::F_PERSISTENT
                                )
             )
          {
            save_state();
          }
          else if (get_owner())
          {
            cleanup (m_path);
          }

          if (m_lock_fd >= 0)
          {
            if (0 != lockf (m_lock_fd, F_ULOCK, 0))
            {
              LLOG (WARN, _logger, "could not unlock beegfs area: " << m_path);
            }
            ::close (m_lock_fd); m_lock_fd = -1;

            fs::remove_all (detail::lock_path (m_path));
          }
        }
      }

      void beegfs_area_t::save_state()
      {
        scoped_file meta ( detail::meta_path (m_path)
                         , O_CREAT | O_EXCL | O_RDWR
                         , 0600
                         );
      }

      void beegfs_area_t::initialize ( path_t const & path
                                  , gpi::pc::type::size_t size
                                  )
      {
        fs::create_directories (path);

        bool success = false;
        FHG_UTIL_FINALLY ([&] { if (!success) { cleanup (path); } });

        {
          // write version information
          fhg::util::write_file ( detail::version_path (path)
                                , version_string()
                                );
        }

        {
          // create data file
          scoped_file data ( detail::data_path (path)
                           , O_CREAT | O_EXCL | O_RDWR
                           , 0600 // TODO: pass in permissions
                           );

          data.ftruncate (size);
        }

        success = true;
      }

      Arena_t
      beegfs_area_t::grow_direction (const gpi::pc::type::flags_t) const
      {
        // we do not support multiple arenas in this memory type
        return ARENA_UP;
      }

      void *
      beegfs_area_t::raw_ptr (gpi::pc::type::offset_t)
      {
        return nullptr;
      }

      bool
      beegfs_area_t::is_allowed_to_attach (const gpi::pc::type::process_id_t) const
      {
        return false;
      }

      void
      beegfs_area_t::alloc_hook (const gpi::pc::type::handle::descriptor_t &hdl)
      {
        if (hdl.creator != (gpi::pc::type::process_id_t)(-1))
        {
          m_topology.alloc ( descriptor ().id
                           , hdl.id
                           , hdl.offset
                           , hdl.size
                           , hdl.local_size
                           , hdl.name
                           );
        }
      }

      void
      beegfs_area_t::free_hook (const gpi::pc::type::handle::descriptor_t &hdl)
      {
        m_topology.free(hdl.id);
      }

      bool
      beegfs_area_t::is_range_local( const gpi::pc::type::handle::descriptor_t &
                                , const gpi::pc::type::offset_t
                                , const gpi::pc::type::offset_t
                                ) const
      {
        return true;
      }

      gpi::pc::type::size_t
      beegfs_area_t::get_local_size ( const gpi::pc::type::size_t size
                                 , const gpi::pc::type::flags_t
                                 ) const
      {
        return size;
      }

      double beegfs_area_t::get_transfer_costs ( const gpi::pc::type::memory_region_t& transfer
                                            , const gpi::rank_t rank
                                            ) const
      {
        return transfer.size;
      }

      gpi::pc::type::size_t
      beegfs_area_t::read_from_impl ( gpi::pc::type::offset_t offset
                                 , void *buffer
                                 , gpi::pc::type::size_t amount
                                 )
      {
        // TODO: implement locking or an fd pool on which we can wait
        int const fd (_fds.get());
        FHG_UTIL_FINALLY ([&] { _fds.put (fd); });
        fhg::util::syscall::lseek (fd, offset, SEEK_SET);
        return fhg::util::syscall::read (fd, buffer, amount);
      }

      gpi::pc::type::size_t
      beegfs_area_t::write_to_impl ( gpi::pc::type::offset_t offset
                                , const void *buffer
                                , gpi::pc::type::size_t amount
                                )
      {
        // TODO: implement locking or an fd pool on which we can wait
        int const fd (_fds.get());
        FHG_UTIL_FINALLY ([&] { _fds.put (fd); });

        struct flock lock_info;
        lock_info.l_type = F_WRLCK;
        lock_info.l_whence = SEEK_SET;
        lock_info.l_start = offset;
        lock_info.l_len = amount;

        fhg::util::syscall::fcntl_setlkw (fd, &lock_info);
        FHG_UTIL_FINALLY ( [&]
                           {
                             lock_info.l_type = F_UNLCK;
                             fhg::util::syscall::fcntl_setlkw (fd, &lock_info);
                           }
                         );
        fhg::util::syscall::lseek (fd, offset, SEEK_SET);
        return fhg::util::syscall::write (fd, buffer, amount);
      }

      area_ptr_t beegfs_area_t::create
        ( fhg::log::Logger& logger
        , std::string const &url_s
        , gpi::pc::global::itopology_t & topology
        , handle_generator_t& handle_generator
        , type::id_t owner
        )
      {
        url_t url (url_s);
        gpi::pc::type::flags_t flags = F_NONE;

        if (not fhg::util::read_bool (url.get ("create").get_value_or ("false")))
        {
          gpi::flag::set (flags, F_NOCREATE);
        }
        if (    fhg::util::read_bool (url.get ("unlink").get_value_or ("false")))
        {
          gpi::flag::set (flags, F_FORCE_UNLINK);
        }
        if (    fhg::util::read_bool (url.get ("exclusive").get_value_or ("false")))
        {
          gpi::flag::set (flags, F_EXCLUSIVE);
        }
        if (    fhg::util::read_bool (url.get ("persistent").get_value_or ("false")))
        {
          gpi::flag::set (flags, F_PERSISTENT);
        }

        gpi::pc::type::size_t size =
          boost::lexical_cast<gpi::pc::type::size_t>(url.get ("total_size").get_value_or ("0"));

        area_ptr_t area (new beegfs_area_t ( logger
                                           , owner
                                           , url.path()
                                           , size
                                           , flags | F_GLOBAL
                                           , topology
                                           , handle_generator
                                           )
                        );
        return area;
      }
    }
  }
}
