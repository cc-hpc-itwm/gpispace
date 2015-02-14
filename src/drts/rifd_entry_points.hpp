// bernd.loerwald@itwm.fraunhofer.de

#pragma once

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
    friend class scoped_runtime_system;

    rifd_entry_points (implementation*);
  };
}
