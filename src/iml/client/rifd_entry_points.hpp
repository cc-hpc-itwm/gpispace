#pragma once

#include <iml/client/rifd_entry_points.fwd.hpp>
#include <iml/client/iml.pimpl.hpp>

#include <boost/filesystem/path.hpp>

namespace iml_client
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
    friend class scoped_iml_runtime_system;

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

    std::string const& hostname() const;

  private:
    friend class scoped_rifd;
    friend class scoped_iml_runtime_system;

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