#include <iml/segment_description.hpp>

#include <utility>

namespace iml
{
  gaspi_segment_description::gaspi_segment_description
      ( std::size_t communication_buffer_size
      , std::size_t communication_buffer_count
      )
    : _communication_buffer_size (communication_buffer_size)
    , _communication_buffer_count (communication_buffer_count)
  {}

  beegfs_segment_description::beegfs_segment_description
      (boost::filesystem::path path)
    : _path (std::move (path))
  {}
}
