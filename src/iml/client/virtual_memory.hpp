#pragma once

#include <iml/client/virtual_memory.fwd.hpp>
#include <iml/client/iml.pimpl.hpp>
#include <iml/client/iml.fwd.hpp>

#include <iml/vmem/gaspi/pc/type/types.hpp>

#include <boost/filesystem/path.hpp>

#include <string>

namespace gpi
{
  namespace pc
  {
    namespace client
    {
      class api_t;
    }
  }
}

namespace iml_client
{
  class scoped_allocation;
  class scoped_iml_runtime_system;

  namespace vmem
  {
    struct gaspi_segment_description
    {
      inline gaspi_segment_description
        ( std::size_t communication_buffer_size = 4 * (1 << 20)
        , std::size_t communication_buffer_count = 8
        );

      std::size_t _communication_buffer_size;
      std::size_t _communication_buffer_count;
    };
    struct beegfs_segment_description
    {
      inline beegfs_segment_description (boost::filesystem::path);

      boost::filesystem::path _path;
    };
  }

  class vmem_allocation
  {
  private:
    friend class scoped_iml_runtime_system;
    friend class stream;

    vmem_allocation ( scoped_iml_runtime_system const* const
                    , vmem::segment_description
                    , unsigned long size
                    , std::string const& description
                    );
    vmem_allocation ( scoped_iml_runtime_system const* const
                    , vmem::segment_description
                    , unsigned long size
                    , std::string const& description
                    , char const* const datia
                    );

  public:
    std::size_t size() const;
    std::unique_ptr<gpi::pc::client::api_t> const& api() const;

    gpi::pc::type::range_t global_memory_range() const;
    gpi::pc::type::range_t global_memory_range ( std::size_t const offset
                                               , std::size_t const size
                                               ) const;

    vmem_allocation (vmem_allocation const&) = delete;
    vmem_allocation& operator= (vmem_allocation const&) = delete;

    vmem_allocation (vmem_allocation&&);
    vmem_allocation& operator= (vmem_allocation&&) = delete;

    PIMPL (vmem_allocation);
  };

  vmem::gaspi_segment_description::gaspi_segment_description
      ( std::size_t communication_buffer_size
      , std::size_t communication_buffer_count
      )
    : _communication_buffer_size (communication_buffer_size)
    , _communication_buffer_count (communication_buffer_count)
  {}

  vmem::beegfs_segment_description::beegfs_segment_description
      (boost::filesystem::path path)
    : _path (std::move (path))
  {}
}