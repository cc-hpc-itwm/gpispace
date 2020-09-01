#pragma once

#include <drts/drts.fwd.hpp>
#include <boost/noncopyable.hpp>

#include <cstdint>
#include <unordered_set>

namespace gspc
{
  class ResourceGraph : boost::noncopyable
  {
  public:
    ResourceGraph();
    ~ResourceGraph();

    std::uint64_t insert
      ( std::string const&
      , std::unordered_set<std::uint64_t> const&
      );

    std::uint64_t insert
      ( std::string const&
      , std::unordered_set<std::uint64_t> const&
      , std::uint64_t const& proxy
      );

    friend class ::gspc::scoped_runtime_system;

  private:
    void* _implementation;
  };
}
