#pragma once

#include <drts/virtual_memory.fwd.hpp>
#include <drts/pimpl.hpp>
#include <drts/drts.fwd.hpp>
#include <drts/stream.fwd.hpp>

#include <we/type/value.hpp>

#include <boost/filesystem/path.hpp>

#include <string>

namespace gspc
{
  class scoped_runtime_system;

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

  class scoped_vmem_segment_and_allocation
  {
  private:
    friend class scoped_runtime_system;
    friend class stream;

    scoped_vmem_segment_and_allocation ( scoped_runtime_system const* const
                                       , vmem::segment_description
                                       , unsigned long size
                                       , std::string const& description
                                       );
    scoped_vmem_segment_and_allocation ( scoped_runtime_system const* const
                                       , vmem::segment_description
                                       , unsigned long size
                                       , std::string const& description
                                       , char const* const datia
                                       );

  public:
    std::size_t size() const;

    pnet::type::value::value_type global_memory_range() const;
    pnet::type::value::value_type global_memory_range ( std::size_t const offset
                                                      , std::size_t const size
                                                      ) const;

    scoped_vmem_segment_and_allocation (scoped_vmem_segment_and_allocation const&) = delete;
    scoped_vmem_segment_and_allocation& operator= (scoped_vmem_segment_and_allocation const&) = delete;

    scoped_vmem_segment_and_allocation (scoped_vmem_segment_and_allocation&&);
    scoped_vmem_segment_and_allocation& operator= (scoped_vmem_segment_and_allocation&&) = delete;

    PIMPL (scoped_vmem_segment_and_allocation);
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
