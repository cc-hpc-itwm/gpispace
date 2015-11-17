#include <gpi-space/pc/memory/sfs_area.hpp>

#include <cerrno>
#include <sys/mman.h> // mmap
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
#include <fhg/util/read_bool.hpp>
#include <util-generic/syscall.hpp>

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
        static sfs_area_t::path_t meta_path (sfs_area_t::path_t const &p)
        {
          return p / "meta";
        }

        static sfs_area_t::path_t data_path (sfs_area_t::path_t const &p)
        {
          return p / "data";
        }

        static sfs_area_t::path_t version_path (sfs_area_t::path_t const &p)
        {
          return p / "version";
        }

        static sfs_area_t::path_t lock_path (sfs_area_t::path_t const &p)
        {
          return p / "lock";
        }

        static std::string my_lock_info ()
        {
          std::ostringstream sstr;
          sstr << fhg::util::hostname() << " " << getpid ();
          return sstr.str ();
        }
      }
      namespace
      {
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

      sfs_area_t::sfs_area_t ( fhg::log::Logger& logger
                             , const gpi::pc::type::process_id_t creator
                             , const sfs_area_t::path_t & path
                             , const gpi::pc::type::size_t size        // total
                             , const gpi::pc::type::flags_t flags
                             , gpi::pc::global::itopology_t & topology
                             , handle_generator_t& handle_generator
                             )
        : area_t ( logger
                 , sfs_area_t::area_type
                 , creator
                 , path.string ()
                 , size
                 , flags
                 , handle_generator
                 )
        , m_ptr (nullptr)
        , m_fd (-1)
        , m_lock_fd (-1)
        , m_path (path)
        , m_version (SFS_VERSION)
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

      sfs_area_t::~sfs_area_t ()
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

      void sfs_area_t::cleanup (sfs_area_t::path_t const &path)
      {
        fs::remove_all (path);
      }

      void sfs_area_t::open()
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
           )
        {
          sfs_area_t::cleanup (m_path);
        }

        if (! fs::exists (m_path) && !gpi::flag::is_set ( descriptor ().flags
                                                        , gpi::pc::F_NOCREATE
                                                        )
           )
        {
          if (0 == m_size)
          {
            throw boost::system::system_error
              (ENOSPC, boost::system::system_category());
          }

          initialize (m_path, m_size);
        }

        // read version information
        {
          path_t vers_path = detail::version_path (m_path);
          FILE *vers_file = fopen (vers_path.string ().c_str (), "r");
          if (vers_file == nullptr)
          {
            throw boost::system::system_error
              (errno, boost::system::system_category());
          }
          int version = -1;
          int rc = fscanf (vers_file, "SFS version %d\n", &version);
          if (rc != 1)
          {
            LLOG (ERROR, _logger, "could not read version information");
            fclose (vers_file);
            throw boost::system::system_error
              (EIO, boost::system::system_category());
          }

          if (version > SFS_VERSION)
          {
            fclose (vers_file);
            throw std::logic_error
              ( "the file segment was created by a newer version:"
                " found: " + std::to_string (version)
              + " wanted: " + std::to_string (SFS_VERSION)
              );
          }

          fclose (vers_file);
        }

        {
          // lock it
          if (m_topology.is_master ())
          {
            static const std::string my_lock_info = detail::my_lock_info ();

            path_t lock_file = detail::lock_path (m_path);

            // check for lock file
            // abort if it exists -> we have it already open?
            m_lock_fd = ::open ( lock_file.string ().c_str ()
                               , O_CREAT + O_RDWR + O_EXCL
                               , S_IRUSR + S_IWUSR
                               );
            if (m_lock_fd < 0)
            {
              // still in use?

              // lock was successful, check who owns/owned it and abort
              int fd = ::open ( lock_file.string ().c_str ()
                              , O_RDONLY
                              );
              if (fd < 0)
              {
                throw boost::system::system_error
                  (errno, boost::system::system_category());
              }

              char buf [1024];
              int read_bytes = ::read (fd, buf, sizeof(buf)-1);
              if (read_bytes)
                buf [read_bytes-1] = '\0';
              else
                buf [0] = '\0';
              ::close (fd); fd = -1;

              // compare lock info
              if (my_lock_info == buf)
              {
                throw std::logic_error ("segment already open");
              }
              else
              {
                std::stringstream sstr (buf);
                std::string other_host;
                sstr >> other_host;

                std::string my_host = fhg::util::hostname ();
                if (other_host == my_host)
                {
                  m_lock_fd = ::open ( lock_file.string ().c_str ()
                                     , O_CREAT + O_RDWR
                                     , S_IRUSR + S_IWUSR
                                     );
                  if (lockf (m_lock_fd, F_TLOCK, 0) < 0)
                  {
                    ::close (m_lock_fd); m_lock_fd = -1;
                    throw std::runtime_error
                      ( "sfs segment still actively in use by another"
                        " process: '" + std::string (buf) + "'"
                      );
                  }
                  else
                  {
                    LLOG (WARN, _logger, "cleaning stale lock file: " << lock_file);

                    fhg::util::syscall::write ( m_lock_fd
                                              , my_lock_info.c_str()
                                              , my_lock_info.size()
                                              );
                    fhg::util::syscall::write (m_lock_fd, "\n", 1);

                    fdatasync (m_lock_fd);
                  }
                }
                else
                {
                  throw std::runtime_error
                    ( "sfs segment may still be in use by: '"
                    + std::string (buf) + "'"
                    + ", if this is wrong, remove " + lock_file.string()
                    );
                }
              }
            }
            else
            {
              if (lockf (m_lock_fd, F_TLOCK, 0) < 0)
              {
                ::close (m_lock_fd); m_lock_fd = -1;
                throw std::logic_error
                  ( "STRANGE: was able to open & create exclusively the"
                    " lock file but not to lock it"
                  );
              }

              fhg::util::syscall::write ( m_lock_fd
                                        , my_lock_info.c_str()
                                        , my_lock_info.size()
                                        );
              fhg::util::syscall::write (m_lock_fd, "\n", 1);

              fdatasync (m_lock_fd);
            }
          }

          // try to open existing file
          path_t data_path = detail::data_path (m_path);
          int fd = ::open ( data_path.string ().c_str ()
                          , O_RDWR
                          );
          if (fd < 0)
          {
            throw boost::system::system_error
              (errno, boost::system::system_category());
          }

          off_t file_size = lseek (fd, 0, SEEK_END);
          if (file_size < 0)
          {
            ::close (fd); fd = -1;
            throw std::runtime_error
              ("could not seek: " + std::string (strerror (errno)));
          }

          lseek (fd, 0, SEEK_SET);
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

          if (not gpi::flag::is_set ( descriptor ().flags
                                    , gpi::pc::F_NOMMAP
                                    )
             )
          {
            m_ptr = mmap ( (void*)nullptr
                         , m_size
                         , PROT_READ + PROT_WRITE
                         , MAP_SHARED
                         , fd
                         , 0
                         );
            if (m_ptr == MAP_FAILED)
            {
              ::close (fd); fd = -1;
              m_ptr = nullptr;
              throw boost::system::system_error
                (errno, boost::system::system_category());
            }
          }

          // we need the fd for read/write support
          m_fd = fd;
        }

        LLOG ( TRACE
             , _logger
             , "SFS memory created:"
             << " path: " << m_path
             << " size: " << m_size
             << " mapped-to: " << m_ptr
             );
      }

      void sfs_area_t::close()
      {
        // only cleanup when we actually opened...
        if ((m_fd >= 0) || (m_ptr != nullptr))
        {
          /*
            steps:

            - check that no ongoing memory transfers are accessing this area
            - unmap memory
            - depending on the flags, remove the directory again
          */
          if (m_fd)
          {
            ::close (m_fd); m_fd = -1;
          }

          if (m_ptr)
          {
            munmap (m_ptr, m_size);
            m_ptr = nullptr;
          }

          if (gpi::flag::is_set ( descriptor ().flags
                                , gpi::pc::F_PERSISTENT
                                )
             )
          {
            save_state();
          }
          else
          {
            cleanup (m_path);
          }

          if (m_lock_fd >= 0)
          {
            if (0 != lockf (m_lock_fd, F_ULOCK, 0))
            {
              LLOG (WARN, _logger, "could not unlock sfs area: " << m_path);
            }
            ::close (m_lock_fd); m_lock_fd = -1;

            fs::remove_all (detail::lock_path (m_path));
          }
        }
      }

      void sfs_area_t::save_state()
      {
        scoped_file meta ( detail::meta_path (m_path)
                         , O_CREAT | O_EXCL | O_RDWR
                         , 0600
                         );
      }

      void sfs_area_t::initialize ( path_t const & path
                                  , gpi::pc::type::size_t size
                                  )
      {
        fs::create_directories (path);

        bool success = false;
        FHG_UTIL_FINALLY ([&] { if (!success) { cleanup (path); } });

        {
          // write version information
          fs::path version_path = detail::version_path (path);
          fs::ofstream ofs (version_path);
          if (! ofs)
          {
            throw boost::system::system_error
              (errno, boost::system::system_category());
          }

          ofs << "SFS version " << SFS_VERSION << std::endl;
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
      sfs_area_t::grow_direction (const gpi::pc::type::flags_t) const
      {
        // we do not support multiple arenas in this memory type
        return ARENA_UP;
      }

      void *
      sfs_area_t::raw_ptr (gpi::pc::type::offset_t off)
      {
        return m_ptr && off < m_size ? (char*)m_ptr + off : nullptr;
      }

      bool
      sfs_area_t::is_allowed_to_attach (const gpi::pc::type::process_id_t) const
      {
        return false;
      }

      void
      sfs_area_t::alloc_hook (const gpi::pc::type::handle::descriptor_t &hdl)
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
      sfs_area_t::free_hook (const gpi::pc::type::handle::descriptor_t &hdl)
      {
        m_topology.free(hdl.id);
      }

      bool
      sfs_area_t::is_range_local( const gpi::pc::type::handle::descriptor_t &
                                , const gpi::pc::type::offset_t
                                , const gpi::pc::type::offset_t
                                ) const
      {
        return true;
      }

      gpi::pc::type::size_t
      sfs_area_t::get_local_size ( const gpi::pc::type::size_t size
                                 , const gpi::pc::type::flags_t
                                 ) const
      {
        return size;
      }

      double sfs_area_t::get_transfer_costs ( const gpi::pc::type::memory_region_t& transfer
                                            , const gpi::rank_t rank
                                            ) const
      {
        return transfer.size;
      }

      gpi::pc::type::size_t
      sfs_area_t::read_from_impl ( gpi::pc::type::offset_t offset
                                 , void *buffer
                                 , gpi::pc::type::size_t amount
                                 )
      {
        // TODO: implement locking or an fd pool on which we can wait
        ::lseek (m_fd, offset, SEEK_SET);
        ssize_t read_rc = ::read (m_fd, buffer, amount);
        if (read_rc < 0)
        {
          throw std::runtime_error ("could not read");
        }
        return (gpi::pc::type::size_t)(read_rc);
      }

      gpi::pc::type::size_t
      sfs_area_t::write_to_impl ( gpi::pc::type::offset_t offset
                                , const void *buffer
                                , gpi::pc::type::size_t amount
                                )
      {
        // TODO: implement locking or an fd pool on which we can wait
        ::lseek (m_fd, offset, SEEK_SET);
        ssize_t write_rc = ::write (m_fd, buffer, amount);
        if (write_rc < 0)
        {
          throw std::runtime_error ("could not write");
        }
        return (gpi::pc::type::size_t)(write_rc);
      }

      area_ptr_t sfs_area_t::create ( fhg::log::Logger& logger
                                    , std::string const &url_s
                                    , gpi::pc::global::itopology_t & topology
                                    , handle_generator_t& handle_generator
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
        if (not fhg::util::read_bool (url.get ("mmap").get_value_or ("false")))
        {
          gpi::flag::set (flags, F_NOMMAP);
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
          boost::lexical_cast<gpi::pc::type::size_t>(url.get ("size").get_value_or ("0"));

        area_ptr_t area (new sfs_area_t ( logger
                                        , GPI_PC_INVAL
                                        , url.path ()
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
