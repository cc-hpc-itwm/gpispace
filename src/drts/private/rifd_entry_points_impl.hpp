// bernd.loerwald@itwm.fraunhofer.de

#include <drts/rifd_entry_points.hpp>

#include <rif/entry_point.hpp>

#include <vector>

namespace gspc
{
  struct rifd_entry_points::implementation
  {
    implementation (std::vector<fhg::rif::entry_point> const& entry_points)
      : _entry_points (entry_points)
    {}

    std::vector<fhg::rif::entry_point> _entry_points;
  };
}
