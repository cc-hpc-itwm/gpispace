#pragma once

#include <boost/filesystem/path.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/variant.hpp>

#include <cstddef>

namespace iml
{
  struct gaspi_segment_description
  {
    gaspi_segment_description
      ( std::size_t communication_buffer_size = 4 * (1 << 20)
      , std::size_t communication_buffer_count = 8
      );

    std::size_t _communication_buffer_size;
    std::size_t _communication_buffer_count;

    template<typename Archive>
      void serialize (Archive& ar, unsigned int);
  };

  struct beegfs_segment_description
  {
    beegfs_segment_description (boost::filesystem::path);

    boost::filesystem::path _path;

    //! \note For serialization only.
    beegfs_segment_description() = default;
    template<typename Archive>
      void serialize (Archive& ar, unsigned int);
  };

  using segment_description
    = boost::variant < gaspi_segment_description
                     , beegfs_segment_description
                     >;
}

#include <iml/segment_description.ipp>
