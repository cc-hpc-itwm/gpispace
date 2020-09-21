#include <iml/fuse/client.hpp>

#include <iml/fuse/detail/Adapter.hpp>

#include <fuse/fuse.h>

#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace iml
{
  namespace fuse
  {
    namespace
    {
      template<typename Memfn, typename... Args, typename... ActualArgs>
        int call (Memfn implementation, ActualArgs&&... args)
      try
      {
        auto const adapter
          (static_cast<detail::Adapter*> (fuse_get_context()->private_data));
        return (adapter->*implementation) (std::forward<ActualArgs> (args)...);
      }
      catch (std::system_error const& error)
      {
        return -error.code().value();
      }
      catch (...)
      {
        return -EOWNERDEAD;
      }

      template<int result, typename... Args>
        int always (Args...)
      {
        return -result;
      }
    }

    //! Mounts and blocks.
    void client ( boost::filesystem::path const& mountpoint
                , boost::filesystem::path const& iml_socket
                )
    {
      detail::Adapter context (iml_socket);

      fuse_operations ops;
      // \todo Just blindly initialize with 0x00? It would be nice if
      // we noticed if there was a missing member. We fixate required
      // version though, so it should never change and a manual
      // verification might be enough.
      std::memset (&ops, 0x00, sizeof (ops));

      // Basic operations.
      ops.open = +[] (char const* path, fuse_file_info* fi)
                  {
                    return call (&detail::Adapter::open, path, fi);
                  };
      ops.read = +[] ( char const*
                     , char* buffer
                     , size_t size
                     , off_t offset
                     , fuse_file_info* fi
                     )
                  {
                    return call ( &detail::Adapter::read
                                , buffer, size, offset, fi
                                );
                  };
      ops.write = +[] ( char const*
                      , char const* buffer
                      , size_t size
                      , off_t offset
                      , fuse_file_info* fi
                      )
                   {
                     return call ( &detail::Adapter::write
                                 , buffer, size, offset, fi
                                 );
                   };
      ops.release = always<0, char const*, fuse_file_info*>;

      // All writes are direct, there is no caching, so there can't be
      // flushing/syncing.
      ops.fsync = always<0, char const*, int, fuse_file_info*>;
      ops.fsyncdir = always<0, char const*, int, fuse_file_info*>;
      ops.flush = always<0, char const*, fuse_file_info*>;

      // \todo List existing segments and allocations.
      ops.getdir = nullptr;
      ops.opendir = nullptr;
      ops.readdir = +[] ( char const* path
                        , void* buffer
                        , fuse_fill_dir_t fill_dir
                        , off_t
                        , fuse_file_info* const
                        )
                     {
                       return call ( &detail::Adapter::readdir
                                   , path, buffer, fill_dir
                                   );
                     };
      ops.releasedir = nullptr;
      ops.fsyncdir = nullptr;
      ops.getattr = +[] (char const* path, struct stat* stat_buf)
                     {
                       return call (&detail::Adapter::getattr, path, stat_buf);
                     };
      ops.fgetattr = nullptr;
      ops.statfs = nullptr;

      // \todo Support segment+allocation creation.
      ops.create = nullptr;
      ops.mkdir = nullptr;
      ops.mknod = nullptr;
      ops.unlink = nullptr;
      ops.rmdir = nullptr;
      ops.rename = nullptr;

      // There is no kind of mapping of allocations to each other, so
      // no links.
      ops.symlink = nullptr;
      ops.link = nullptr;
      ops.readlink = nullptr;

      // Allocation size can't be modified after allocation.
      ops.ftruncate = always<EPERM, char const*, off_t, fuse_file_info*>;

      // Access control is not supported.
      ops.chmod = nullptr;
      ops.chown = nullptr;
      ops.access = nullptr;
      // \todo set default_permissions mount option or not?

      // Xattrs are not supported.
      ops.getxattr = nullptr;
      ops.listxattr = nullptr;
      ops.removexattr = nullptr;
      ops.setxattr = nullptr;

      // Locking is not supported. When not implemented, the kernel allows
      // local locking, which we don't want either.
      // \todo It apparently STILL locks, and claims to have success?!
      ops.lock = always<ENOLCK, char const*, fuse_file_info*, int, flock*>;

      // No timestamps supported.
      ops.utime = nullptr;
      ops.utimens = nullptr;
      ops.flag_utime_omit_ok = 0;

      // Not a blkdev.
      ops.bmap = nullptr;

      // Paths for read/write are not required as everything is in the
      // handle. `nullpath_ok` talks about "after unlinking", but we
      // don't support unlinking. Set both in case they do some
      // optimization.
      ops.flag_nullpath_ok = 0 /* \todo readdir needs path */;
      ops.flag_nopath = 0 /* \todo readdir needs path */;

      // Meta-operations: We pass user_data via fuse_main so not needed.
      ops.init = nullptr;
      ops.destroy = nullptr;

      // 2.8+
      // ops.ioctl = nullptr;
      // ops.poll = nullptr;

      // 2.9+
      // ops.write_buf = nullptr;
      // ops.read_buf = nullptr;
      // ops.flock = nullptr;

      // 2.9.1+
      // ops.fallocate = nullptr;

      std::vector<char> fuse_argv_buffer;
      std::vector<std::size_t> fuse_argv_positions;
      for ( std::string arg
          : { "iml-fuse-client"
            , mountpoint.c_str()
            , "-f"
            , "-d"
            , "-o", "auto_unmount" // auto unmount on process termination
            // \todo This means it compares stat() results before calling
            // us. We currently don't have any access control at all, so
            // probably keep disabled?
            //, "-o", "default_permissions" // enable permission checking by kernel
            , "-o", "fsname=fhgiml"
            // \todo Enough to set on open?
            // , "-o", "direct_io"
            // \todo Caching?
            , "-o", "no_remote_lock" // disable remote file locking
            , "-o", "no_remote_flock" // disable remote file locking (BSD)
            , "-o", "no_remote_posix_lock" // disable remove file locking (POSIX)
            }
          )
      {
        fuse_argv_positions.emplace_back (fuse_argv_buffer.size());
        fuse_argv_buffer.insert
          (fuse_argv_buffer.end(), arg.begin(), arg.end());
        fuse_argv_buffer.emplace_back ('\0');
      }

      std::vector<char*> fuse_argv;
      for (auto const& pos : fuse_argv_positions)
      {
        fuse_argv.emplace_back (fuse_argv_buffer.data() + pos);
      }

      if (fuse_main (fuse_argv.size(), fuse_argv.data(), &ops, &context) != 0)
      {
        throw std::runtime_error ("fuse_main failed");
      }
    }
  }
}
