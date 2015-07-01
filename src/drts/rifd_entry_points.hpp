#pragma once

#include <drts/rifd_entry_points.fwd.hpp>
#include <drts/pimpl.hpp>

#include <boost/filesystem/path.hpp>

namespace gspc
{
  class rifd_entry_points
  {
  public:
    rifd_entry_points (boost::filesystem::path const&);

    rifd_entry_points (rifd_entry_points const&);

    void write_to_file (boost::filesystem::path const&);

  private:
    friend class scoped_rifds;
    friend class scoped_runtime_system;

    PIMPL (rifd_entry_points);

    rifd_entry_points (implementation*);
  };

  class rifd_entry_point
  {
  private:
    friend class scoped_rifd;
    friend class scoped_runtime_system;

    PIMPL (rifd_entry_point);

    rifd_entry_point (implementation*);
  };
}
