#include <iml/client/rifd_entry_points.hpp>
#include <iml/rif/entry_point.hpp>

#include <vector>

namespace iml_client
{
  struct rifd_entry_points::implementation
  {
    implementation (std::vector<fhg::iml::rif::entry_point> const& entry_points)
      : _entry_points (entry_points)
    {}

    std::vector<fhg::iml::rif::entry_point> _entry_points;
  };

  struct rifd_entry_point::implementation
  {
    implementation (fhg::iml::rif::entry_point const& entry_point)
      : _entry_point (entry_point)
    {}
    std::string const& hostname() const
    {
      return _entry_point.hostname;
    }

    fhg::iml::rif::entry_point _entry_point;
  };
}