// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_GSPC_RIFD_ENTRY_POINTS_HPP
#define FHG_GSPC_RIFD_ENTRY_POINTS_HPP

#include <drts/rifd_entry_points.fwd.hpp>

#include <boost/filesystem/path.hpp>

namespace gspc
{
  class rifd_entry_points
  {
  public:
    rifd_entry_points (boost::filesystem::path const&);
    ~rifd_entry_points();

    rifd_entry_points (rifd_entry_points const&);

    void write_to_file (boost::filesystem::path const&);

  private:
    struct implementation;
    implementation* _;
    friend class scoped_rifd;

    rifd_entry_points (implementation*);
  };
}

#endif
