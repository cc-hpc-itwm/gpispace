#pragma once

#include <iml/vmem/gaspi/pc/type/handle.hpp>
#include <iml/vmem/gaspi/pc/type/types.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/variant/variant.hpp>

namespace iml
{
  using SegmentHandle = gpi::pc::type::segment_id_t;
  using AllocationHandle = gpi::pc::type::handle_t;

  namespace fuse
  {
    namespace detail
    {
      using Unknown = boost::filesystem::path;
      struct TopLevel{};
      using PathOrHandleBase = boost::variant< Unknown
                                             , TopLevel
                                             , SegmentHandle
                                             , AllocationHandle
                                             >;
      struct PathOrHandle : PathOrHandleBase
      {
        //! /                               top level
        //! /segmentid                      segment
        //! /segmentid/                     segment
        //! /segmentid/allocationid         allocation
        // \todo no ../s taken into account.
        PathOrHandle (char const* path);
      };
    }
  }
}
