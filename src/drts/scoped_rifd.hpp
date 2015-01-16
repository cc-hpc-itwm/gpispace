// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_GSPC_SCOPED_RIFD_HPP
#define FHG_GSPC_SCOPED_RIFD_HPP

#include <drts/scoped_rifd.fwd.hpp>

#include <drts/rifd_entry_points.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include <string>
#include <vector>

namespace gspc
{
  class scoped_rifd
  {
  public:
    scoped_rifd ( std::string const& strategy
                , std::vector<std::string> const& hostnames
                , boost::optional<unsigned short> const& rifd_port
                , boost::filesystem::path const& gspc_home
                );
    ~scoped_rifd();

    rifd_entry_points entry_points() const;

  private:
    struct implementation;
    implementation* _;
  };
}

#endif
