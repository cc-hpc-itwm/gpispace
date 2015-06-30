// bernd.loerwald@itwm.fraunhofer.de

#include <drts/rifd_entry_points.hpp>

#include <rif/entry_point.hpp>

#include <list>
#include <unordered_set>
#include <vector>

namespace gspc
{
  struct rifd_entry_points::implementation
  {
    implementation (std::vector<fhg::rif::entry_point> const& entry_points)
      : _entry_points (entry_points)
    {}

    std::pair<std::list<std::string>, unsigned long>
      nodes_and_number_of_unique_nodes() const
    {
      std::unordered_set<std::string> unique_nodes;
      std::list<std::string> nodes;

      for (fhg::rif::entry_point const& entry_point : _entry_points)
      {
        unique_nodes.emplace (entry_point.hostname);
        nodes.emplace_back (entry_point.hostname);
      }

      return {nodes, unique_nodes.size()};
    }

    std::vector<fhg::rif::entry_point> _entry_points;
  };

  struct rifd_entry_point::implementation
  {
    implementation (fhg::rif::entry_point const& entry_point)
      : _entry_point (entry_point)
    {}

    fhg::rif::entry_point _entry_point;
  };
}
