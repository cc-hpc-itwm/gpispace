#pragma once

#include <iml/client/scoped_shm_allocation.hpp>
#include <iml/fuse/detail/PathOrHandle.hpp>
#include <iml/vmem/gaspi/pc/client/api.hpp>

#include <util-generic/threadsafe_queue.hpp>

#include <boost/filesystem/path.hpp>

// \todo Hide from public(?) API.
#include <fuse/fuse.h>

#include <cstddef>

namespace iml
{
  using Client = gpi::pc::client::api_t;

  namespace client
  {
    using ScopedSHMAllocation = scoped_shm_allocation;
  }

  namespace fuse
  {
    namespace detail
    {
      class Adapter
      {
      public:
        Adapter (boost::filesystem::path const& iml_socket);

        int getattr (PathOrHandle const&, struct stat*) const;
        int readdir (PathOrHandle const&, void*, fuse_fill_dir_t) const;
        int open (PathOrHandle const&, fuse_file_info*) const;
        int read (char*, std::size_t, std::size_t, fuse_file_info const*);
        int write (char const*, std::size_t, std::size_t, fuse_file_info const*);

      private:
        iml::Client _client;
        fhg::util::threadsafe_queue<client::ScopedSHMAllocation> _buffers;
      };
    }
  }
}
