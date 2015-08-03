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
    friend class rifds;
    friend class scoped_rifds;
    friend class scoped_runtime_system;

    PIMPL (rifd_entry_points);

    rifd_entry_points (implementation*);
  };

  class rifd_entry_point
  {
  private:
    PIMPL (rifd_entry_point);

  public:
    rifd_entry_point (implementation*);
    rifd_entry_point (rifd_entry_point&&);

  private:
    friend class scoped_rifd;
    friend class scoped_runtime_system;

    rifd_entry_point (rifd_entry_point const&) = delete;
    rifd_entry_point& operator= (rifd_entry_point&&) = delete;
    rifd_entry_point& operator= (rifd_entry_point const&) = delete;

    friend struct rifd_entry_point_hash;
    friend bool operator== (rifd_entry_point const&, rifd_entry_point const&);
  };

  struct rifd_entry_point_hash
  {
    std::size_t operator() (rifd_entry_point const&) const;
  };
  bool operator== (rifd_entry_point const&, rifd_entry_point const&);
}
