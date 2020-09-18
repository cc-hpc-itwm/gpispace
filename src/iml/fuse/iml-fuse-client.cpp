#include <iml/client/scoped_shm_allocation.hpp>
#include <iml/vmem/gaspi/pc/client/api.hpp>

#include <util-generic/functor_visitor.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/threadsafe_queue.hpp>

#include <boost/filesystem/path.hpp>

#include <fuse/fuse.h>

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

namespace iml
{
  using Client = gpi::pc::client::api_t;
  using AllocationHandle = gpi::pc::type::handle_t;

  namespace client
  {
    using ScopedSHMAllocation = scoped_shm_allocation;
  }

  namespace fuse
  {
    namespace detail
    {
      struct PathOrHandle
        : boost::variant<boost::filesystem::path, AllocationHandle>
      {
        using Base = boost::variant<boost::filesystem::path, AllocationHandle>;
        PathOrHandle (boost::filesystem::path const& path);
        PathOrHandle (char const* path);
      };

      PathOrHandle::PathOrHandle (char const* path)
        : PathOrHandle (boost::filesystem::path (path))
      {}

      PathOrHandle::PathOrHandle (boost::filesystem::path const& path)
        : Base (path)
      {
        try
        {
          auto const& path_string (path.string());
          if (path_string.size() < 2 || path_string[0] != '/')
          {
            return;
          }

          std::istringstream iss (path_string);
          iss.get();

          AllocationHandle handle;
          iss >> handle;
          static_cast<Base&> (*this) = std::move (handle);
        }
        catch (...)
        {
        }
      }
    }

    class Adapter
    {
    public:
      Adapter (boost::filesystem::path const& iml_socket);

      int getattr (detail::PathOrHandle const&, struct stat*) const;
      int open (detail::PathOrHandle const&, fuse_file_info*) const;
      int read (char*, std::size_t, std::size_t, fuse_file_info const*);
      int write (char const*, std::size_t, std::size_t, fuse_file_info const*);

    private:
      iml::Client _client;
      fhg::util::threadsafe_queue<client::ScopedSHMAllocation> _buffers;
    };

    Adapter::Adapter (boost::filesystem::path const& iml_socket)
      : _client (iml_socket.string())
    {
      // \todo Tune: Number of buffers.
      // \todo Tune: Size of buffers.
      for (int i (0); i < 16; ++i)
      {
        _buffers.put (&_client, "does this matter?!", 16 * 1 << 20 /*M*/);
      }
    }

    int Adapter::getattr ( detail::PathOrHandle const& path_or_handle
                         , struct stat* const stat_buf
                         ) const
    {
      std::memset (stat_buf, 0, sizeof (*stat_buf));

      return fhg::util::visit<int>
        ( path_or_handle
        , [&] (boost::filesystem::path const& path)
          {
            if (path == "/")
            {
              stat_buf->st_mode = S_IFDIR | S_IRUSR | S_IWUSR | S_IXUSR;
              stat_buf->st_nlink = 2;
              // \todo Fake entries with a readme.
              return 0;
            }
            return -ENOENT;
          }
        , [&] (AllocationHandle const&)
          {
            // \todo Should we claim we're a block device instead?
            stat_buf->st_mode = S_IFREG | S_IRUSR | S_IWUSR;
            stat_buf->st_nlink = 1;
            // \todo Query size.
            // stat_buf->st_size = info (handle).size;
            return 0;
          }
        );
  //              uid_t     st_uid;     /* user ID of owner */
  //              gid_t     st_gid;     /* group ID of owner */
  //              dev_t     st_rdev;    /* device ID (if special file) */
  //              off_t     st_size;    /* total size, in bytes */
  //              blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
  //              time_t    st_atime;   /* time of last access */
  //              time_t    st_mtime;   /* time of last modification */
  //              time_t    st_ctime;   /* time of last status change */
    }

    int Adapter::open ( detail::PathOrHandle const& path_or_handle
                      , fuse_file_info* const fi
                      ) const
    {
      static_assert ( sizeof (decltype (fi->fh)) == sizeof (AllocationHandle)
                    , "fuse file handle shall match iml alloc handle"
                    );

      return fhg::util::visit<int>
        ( path_or_handle
        , [&] (boost::filesystem::path const&)
          {
            return -ENOSYS;
          }
        , [&] (AllocationHandle const& handle)
          {
            // \todo Do we need to set this when we used the mount option?
            fi->direct_io = 1;
            fi->fh = handle;
            return 0;
          }
        );
    }

    int Adapter::read ( char* buffer
                      , std::size_t const size
                      , std::size_t offset
                      , fuse_file_info const* const fi
                      )
    {
      AllocationHandle const handle (fi->fh);

      std::size_t left_to_read (size);

      // \todo get_up_to (#parts);
      while (left_to_read)
      {
        decltype (_buffers)::scoped_backout const buffer_backout (_buffers);
        client::ScopedSHMAllocation const& shm_buffer (buffer_backout);

        auto const to_read (std::min (shm_buffer.size(), left_to_read));

        _client.memcpy_and_wait ({shm_buffer, 0}, {handle, offset}, to_read);

        std::memcpy (buffer, _client.ptr (shm_buffer), to_read);

        offset += to_read;
        buffer += to_read;
        left_to_read -= to_read;
      }

      return size;
    }

    int Adapter::write ( char const* buffer
                       , std::size_t const size
                       , std::size_t offset
                       , fuse_file_info const* const fi
                       )
    {
      AllocationHandle const handle (fi->fh);

      std::size_t left_to_write (size);

      // \todo get_up_to (#parts);
      while (left_to_write)
      {
        decltype (_buffers)::scoped_backout const buffer_backout (_buffers);
        client::ScopedSHMAllocation const& shm_buffer (buffer_backout);

        auto const to_write (std::min (shm_buffer.size(), left_to_write));

        std::memcpy (_client.ptr (shm_buffer), buffer, to_write);

        _client.memcpy_and_wait ({handle, offset}, {shm_buffer, 0}, to_write);

        offset += to_write;
        buffer += to_write;
        left_to_write -= to_write;
      }

      return size;
    }
  }
}

namespace wrapper
{
  namespace
  {
    template<typename... Args, typename... ActualArgs>
      int call (int (iml::fuse::Adapter::* implementation) (Args...) const, ActualArgs&&... args)
    try
    {
      auto const adapter
        (static_cast<iml::fuse::Adapter const*> (fuse_get_context()->private_data));
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

    template<typename... Args, typename... ActualArgs>
      int call (int (iml::fuse::Adapter::* implementation) (Args...), ActualArgs&&... args)
    try
    {
      auto const adapter
        (static_cast<iml::fuse::Adapter*> (fuse_get_context()->private_data));
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

    int open (char const* path, fuse_file_info* fi)
    {
      return call (&iml::fuse::Adapter::open, path, fi);
    }

    int read ( char const*
             , char* buffer
             , size_t size
             , off_t offset
             , fuse_file_info* fi
             )
    {
      return call (&iml::fuse::Adapter::read, buffer, size, offset, fi);
    }
    int write ( char const*
              , char const* buffer
              , size_t size
              , off_t offset
              , fuse_file_info* fi
              )
    {
      return call (&iml::fuse::Adapter::write, buffer, size, offset, fi);
    }

    int getattr (char const* path, struct stat* stat_buf)
    {
      return call (&iml::fuse::Adapter::getattr, path, stat_buf);
    }
  }
}

namespace
{
  template<int result, typename... Args>
    int always (Args...)
  {
    return -result;
  }
}

int main (int argc, char** argv)
try
{
  // \todo Command line parsing.
  if (argc != 3)
  {
    throw std::invalid_argument ("usage: <exe> mountpoint iml-socket");
  }
  std::string const mountpoint (argv[1]);
  std::string const iml_socket (argv[2]);

  iml::fuse::Adapter context (iml_socket);

  fuse_operations ops;
  // \todo Just blindly initialize with 0x00? It would be nice if we
  // noticed if there was a missing member. We fixate required version
  // though, so it should never change and a manual verification might
  // be enough.
  std::memset (&ops, 0x00, sizeof (ops));

  // Basic operations.
  ops.open = wrapper::open;
  ops.read = wrapper::read;
  ops.write = wrapper::write;
  ops.release = always<0, char const*, fuse_file_info*>;

  // All writes are direct, there is no caching, so there can't be
  // flushing/syncing.
  ops.fsync = always<0, char const*, int, fuse_file_info*>;
  ops.fsyncdir = always<0, char const*, int, fuse_file_info*>;
  ops.flush = always<0, char const*, fuse_file_info*>;

  // \todo List existing segments and allocations.
  ops.getdir = nullptr;
  ops.opendir = nullptr;
  ops.readdir = nullptr;
  ops.releasedir = nullptr;
  ops.fsyncdir = nullptr;
  ops.getattr = wrapper::getattr;
  ops.fgetattr = nullptr;
  ops.statfs = nullptr;

  // \todo Support segment+allocation creation.
  ops.create = nullptr;
  ops.mkdir = nullptr;
  ops.mknod = nullptr;
  ops.unlink = nullptr;
  ops.rmdir = nullptr;
  ops.rename = nullptr;

  // There is no kind of mapping of allocations to each other, so no links.
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
  // handle. `nullpath_ok` talks about "after unlinking", but we don't
  // support unlinking. Set both in case they do some optimization.
  ops.flag_nullpath_ok = 1;
  ops.flag_nopath = 1;

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
    fuse_argv_buffer.insert (fuse_argv_buffer.end(), arg.begin(), arg.end());
    fuse_argv_buffer.emplace_back ('\0');
  }

  std::vector<char*> fuse_argv;
  for (auto const& pos : fuse_argv_positions)
  {
    fuse_argv.emplace_back (fuse_argv_buffer.data() + pos);
  }

  return fuse_main (fuse_argv.size(), fuse_argv.data(), &ops, &context);
}
catch (...)
{
  std::cerr << fhg::util::current_exception_printer() << "\n";
  return 1;
}
