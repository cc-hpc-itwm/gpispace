// bernd.loerwald@itwm.fraunhofer.de

#include <drts/rifd_entry_points.hpp>

#include <drts/private/pimpl.hpp>
#include <drts/private/rifd_entry_points_impl.hpp>

#include <util-generic/read_lines.hpp>

#include <fstream>

namespace gspc
{
  namespace
  {
    std::vector<fhg::rif::entry_point> parse (boost::filesystem::path const& path)
    {
      std::vector<std::string> const lines (fhg::util::read_lines (path));
      return {lines.begin(), lines.end()};
    }
  }

  rifd_entry_points::rifd_entry_points (boost::filesystem::path const& path)
    : rifd_entry_points (new implementation (parse (path)))
  {}
  rifd_entry_points::rifd_entry_points (implementation* impl)
    : _ (impl)
  {}
  PIMPL_DTOR (rifd_entry_points)

  rifd_entry_points::rifd_entry_points (rifd_entry_points const& other)
    : _ (new implementation (*other._))
  {}

  void rifd_entry_points::write_to_file (boost::filesystem::path const& path)
  {
    std::ofstream file (path.string());
    if (!file)
    {
      throw std::runtime_error ("unable to open file " + path.string());
    }
    for (fhg::rif::entry_point const& entry_point : _->_entry_points)
    {
      if (!(file << entry_point.to_string() << '\n'))
      {
        throw std::runtime_error ("unable to write to file " + path.string());
      }
    }
  }

  rifd_entry_point::rifd_entry_point (implementation* impl)
    : _ (impl)
  {}
  PIMPL_DTOR (rifd_entry_point);
}
