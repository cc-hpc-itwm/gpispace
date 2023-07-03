// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/vmem/gaspi/pc/memory/beegfs_area.hpp>

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <util-generic/finally.hpp>
#include <util-generic/hostname.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/read_file.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/unused.hpp>
#include <util-generic/write_file.hpp>

#include <iml/vmem/segment/beegfs.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/optional.hpp>
#include <boost/system/system_error.hpp>

#include <memory>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      namespace detail
      {
        static beegfs_area_t::path_t data_path (beegfs_area_t::path_t const& p)
        {
          return p / "data";
        }

        static beegfs_area_t::path_t version_path (beegfs_area_t::path_t const& p)
        {
          return p / "version";
        }

        static beegfs_area_t::path_t lock_path (beegfs_area_t::path_t const& p)
        {
          return p / "lock";
        }
        namespace
        {
          ::boost::filesystem::path lock_info_path (::boost::filesystem::path const& path)
          {
            return lock_path (path) / "info";
          }

          struct lock_info
          {
            lock_info (::boost::filesystem::path const& path)
            {
              std::string const content
                (fhg::util::read_file (detail::lock_info_path (path)));
              std::istringstream iss (content);
              ::boost::archive::text_iarchive ia (iss);
              ia & hostname;
              ia & pid;
            }

            static void write_local_information
              (::boost::filesystem::path const& path)
            {
              std::ostringstream oss;
              ::boost::archive::text_oarchive oa (oss);
              oa & fhg::util::hostname();
              oa & fhg::util::syscall::getpid();
              fhg::util::write_file
                (detail::lock_info_path (path), oss.str());
            }

            std::string hostname;
            pid_t pid;
          };
        }
      }
      namespace
      {
        struct scoped_file
        {
          scoped_file ( ::boost::filesystem::path const& path
                      , int flags
                      , unsigned int perms
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

          scoped_file (scoped_file const&) = delete;
          scoped_file& operator= (scoped_file const&) = delete;
          scoped_file (scoped_file&&) = delete;
          scoped_file& operator= (scoped_file&&) = delete;

          int _fd;
        };
      }

      beegfs_area_t::beegfs_area_t ( bool is_creator
                                   , beegfs_area_t::path_t const& path
                                   , iml::MemorySize size        // total
                                   , gpi::pc::global::itopology_t & topology
                                   )
        : area_t (size)
        , _is_creator (is_creator)
        , m_path (path)
        , m_version (BEEGFS_AREA_VERSION)
        , m_topology (topology)
      {
        try
        {
          open (size);
        }
        catch (...)
        {
          std::throw_with_nested
            (std::runtime_error ("opening '" + path.string() + "' failed"));
        }
      }

      beegfs_area_t::~beegfs_area_t ()
      {
        try
        {
          close();
        }
        catch (...)
        { }
      }

      std::string beegfs_area_t::version_string() const
      {
        return "BEEGFS segment version " + std::to_string (m_version);
      }

      beegfs_area_t::lock_file_helper::lock_file_helper
        (beegfs_area_t& area)
      try
        : filesystem_lock_directory (detail::lock_path (area.m_path))
      {
        detail::lock_info::write_local_information (area.m_path);
      }
      catch (...)
      {
        ::boost::optional<std::string> additional_information;
        try
        {
          detail::lock_info const existing (area.m_path);
          additional_information
            = " (existing lock was created by pid "
            + std::to_string (existing.pid)
            + " on host " + existing.hostname + ")";
        }
        catch (...)
        {
          //! \note ignore: just directory existed but no info file,
          //! or reading it failed due to being corrupt or the lock
          //! being removed the exact same time we tried to
          //! lock. either way, we can't give additional information
          //! to the user
        }

        std::throw_with_nested
          ( std::runtime_error
              ( "segment is still locked or creating lock at directory "
              + static_cast<::boost::filesystem::path const&> (*this).string()
              + " failed" + additional_information.get_value_or ("")
              )
          );
      }

      void beegfs_area_t::open (std::size_t total_size)
      {
        if (_is_creator)
        {
          if (::boost::filesystem::exists (m_path))
          {
            throw std::runtime_error ( "unable to create BeeGFS segment: "
                                     + m_path.string() + " already exists"
                                     );
          }

          ::boost::filesystem::create_directories (m_path);

          {
            fhg::util::write_file ( detail::version_path (m_path)
                                  , version_string()
                                  );
          }

          {
            scoped_file data ( detail::data_path (m_path)
                             , O_CREAT | O_EXCL | O_RDWR
                             , 0600 // TODO: pass in permissions
                             );

            data.ftruncate (total_size);
          }
        }
        bool succeeded (false);
        FHG_UTIL_FINALLY
          ( [&]
            {
              if (!succeeded && _is_creator)
              {
                ::boost::filesystem::remove (detail::data_path (m_path));
                ::boost::filesystem::remove (detail::version_path (m_path));
                ::boost::filesystem::remove (m_path);
              }
            }
          );

        //! \todo move in front of creation even by checking first
        //! existing parent path
        fhg::iml::vmem::segment::beegfs::check_requirements (m_path);

        {
          auto const found_version
            ( [&]() -> ::boost::optional<std::string>
              {
                try
                {
                  return fhg::util::read_file<std::string>
                    (detail::version_path (m_path));
                }
                catch (...)
                {
                  return ::boost::none;
                }
              }()
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

        if (_is_creator)
        {
          _lock_file = std::make_unique<lock_file_helper> (*this);
        }
        FHG_UTIL_FINALLY ([&] { if (!succeeded) { _lock_file.reset(); }});

        path_t data_path = detail::data_path (m_path);
        int const fd ( fhg::util::syscall::open
                         (data_path.string ().c_str(), O_RDWR)
                     );
        FHG_UTIL_FINALLY ([&] { fhg::util::syscall::close (fd); });

        off_t const file_size (fhg::util::syscall::lseek (fd, 0, SEEK_END));
        fhg::util::syscall::lseek (fd, 0, SEEK_SET);

        if (total_size != static_cast<iml::MemorySize> (file_size))
        {
          throw std::logic_error
            ( "segment file on disk has size "
            + std::to_string (file_size)
            + " but tried to open it with size "
            + std::to_string (total_size)
            );
        }

        //! \todo better constant (number of threads? configurable?)
        for (int i (0); i < 20; ++i)
        {
          _fds.put
            (fhg::util::syscall::open (data_path.string().c_str(), O_RDWR));
        }

        succeeded = true;
      }

      void beegfs_area_t::close()
      {
        if (_is_creator)
        {
          ::boost::filesystem::remove (detail::data_path (m_path));
          ::boost::filesystem::remove (detail::version_path (m_path));

          _lock_file.reset();

          ::boost::filesystem::remove (m_path);
        }
      }

      void *
      beegfs_area_t::raw_ptr (iml::MemoryOffset)
      {
        return nullptr;
      }

      global::itopology_t& beegfs_area_t::global_topology()
      {
        return m_topology;
      }

      bool
      beegfs_area_t::is_range_local( gpi::pc::type::handle::descriptor_t const&
                                , iml::MemoryOffset
                                , iml::MemoryOffset
                                ) const
      {
        return true;
      }

      iml::MemorySize
      beegfs_area_t::get_local_size ( iml::MemorySize size
                                 , gpi::pc::type::flags_t
                                 ) const
      {
        return size;
      }

      double beegfs_area_t::get_transfer_costs
        ( iml::MemoryRegion const& transfer
        , gpi::rank_t FHG_UTIL_UNUSED
                            ( rank
                            , "NYI: use BeeGFS::beegfs_getStripeTarget"
                              "to get locality information"
                            )
        ) const
      {
        return transfer.size;
      }

      iml::MemorySize
      beegfs_area_t::read_from_impl ( iml::MemoryOffset offset
                                 , void *buffer
                                 , iml::MemorySize amount
                                 )
      {
        int const fd (_fds.get());
        FHG_UTIL_FINALLY ([&] { _fds.put (fd); });
        fhg::util::syscall::lseek (fd, offset, SEEK_SET);
        return fhg::util::syscall::read (fd, buffer, amount);
      }

      iml::MemorySize
      beegfs_area_t::write_to_impl ( iml::MemoryOffset offset
                                , const void *buffer
                                , iml::MemorySize amount
                                )
      {
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
        ( iml::beegfs::SegmentDescription const& description
        , unsigned long total_size
        , gpi::pc::global::itopology_t & topology
        , bool is_creator
        )
      {
        area_ptr_t area (new beegfs_area_t ( is_creator
                                           , description.path
                                           , total_size
                                           , topology
                                           )
                        );
        return area;
      }
    }
  }
}
