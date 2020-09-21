#include <iml/fuse/detail/Adapter.hpp>

#include <util-generic/divru.hpp>
#include <util-generic/functor_visitor.hpp>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

namespace iml
{
  namespace fuse
  {
    namespace detail
    {
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

      int Adapter::getattr ( PathOrHandle const& path_or_handle
                           , struct stat* const stat_buf
                           ) const
      {
        std::memset (stat_buf, 0, sizeof (*stat_buf));

        stat_buf->st_uid = fuse_get_context()->uid;
        stat_buf->st_gid = fuse_get_context()->gid;

        return fhg::util::visit<int>
          ( path_or_handle
          , [&] (Unknown const&)
            {
              return -ENOENT;
            }
          , [&] (TopLevel const&)
            {
              stat_buf->st_mode = S_IFDIR | S_IRUSR | S_IWUSR | S_IXUSR;
              stat_buf->st_nlink = 2 + _client.existing_segments().size();
              // \todo Fake entries with a readme.
              return 0;
            }
          , [&] (SegmentHandle const& segment)
            {
              stat_buf->st_mode = S_IFDIR | S_IRUSR | S_IWUSR | S_IXUSR;
              stat_buf->st_nlink
                = 2 + _client.existing_allocations (segment).size();
              return 0;
            }
          , [&] (AllocationHandle const& handle)
            {
              try
              {
                auto const info (_client.stat (handle));
                stat_buf->st_mode = S_IFREG | S_IRUSR | S_IWUSR;
                stat_buf->st_nlink = 1;
                stat_buf->st_size = info.size;
                stat_buf->st_blocks = fhg::util::divru (info.size, 512ul);
                return 0;
              }
              catch (...)
              {
                return -ENOENT;
              }
            }
          );
    //              dev_t     st_rdev;    /* device ID (if special file) */
    //              time_t    st_atime;   /* time of last access */
    //              time_t    st_mtime;   /* time of last modification */
    //              time_t    st_ctime;   /* time of last status change */
      }

      int Adapter::readdir ( PathOrHandle const& path_or_handle
                           , void* buffer
                           , fuse_fill_dir_t fill_dir
                           ) const
      {
  #define add(path_)                                      \
        do                                                \
        {                                                 \
          if (fill_dir (buffer, path_, nullptr, 0))       \
          {                                               \
            return -EINVAL;                               \
          }                                               \
        }                                                 \
        while (false)

        return fhg::util::visit<int>
          ( path_or_handle
          , [&] (Unknown const&)
            {
              return -ENOENT;
            }
          , [&] (TopLevel const&)
            {
              add (".");
              add ("..");
              for (auto const& elem : _client.existing_segments())
              {
                std::ostringstream oss;
                oss << elem;
                add (oss.str().c_str());
              }
              return 0;
            }
          , [&] (SegmentHandle const& segment)
            {
              add (".");
              add ("..");
              for ( gpi::pc::type::handle_t elem
                  : _client.existing_allocations (segment)
                  )
              {
                std::ostringstream oss;
                oss << elem;
                add (oss.str().c_str());
              }
              return 0;
            }
          , [&] (AllocationHandle const&)
            {
              return -ENOTDIR;
            }
          );

  #undef add
      }

      int Adapter::open ( PathOrHandle const& path_or_handle
                        , fuse_file_info* const fi
                        ) const
      {
        static_assert ( sizeof (decltype (fi->fh)) == sizeof (AllocationHandle)
                      , "fuse file handle shall match iml alloc handle"
                      );

        return fhg::util::visit<int>
          ( path_or_handle
          , [&] (Unknown const&)
            {
              return -ENOSYS;
            }
          , [&] (TopLevel const&)
            {
              return -ENOSYS;
            }
          , [&] (SegmentHandle const&)
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

      namespace
      {
        void adjust_to_allocation_boundaries
          ( Client::AllocationInformation const& info
          , std::size_t& size
          , std::size_t& offset
          )
        {
          offset = std::min (info.size, offset);
          size = std::min (info.size - offset, size);
        }
      }

      int Adapter::read ( char* buffer
                        , std::size_t size
                        , std::size_t offset
                        , fuse_file_info const* const fi
                        )
      {
        AllocationHandle const handle (fi->fh);

        adjust_to_allocation_boundaries (_client.stat (handle), size, offset);
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
                         , std::size_t size
                         , std::size_t offset
                         , fuse_file_info const* const fi
                         )
      {
        AllocationHandle const handle (fi->fh);

        adjust_to_allocation_boundaries (_client.stat (handle), size, offset);
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
}
