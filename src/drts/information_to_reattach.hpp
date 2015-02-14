// alexander.petry@itwm.fraunhofer.de

#pragma once

#include <drts/drts.fwd.hpp>

#include <string>
#include <memory>

namespace gspc
{
  class information_to_reattach
  {
  public:
    information_to_reattach (scoped_runtime_system const&);
    information_to_reattach (std::string const&);
    ~information_to_reattach();

    std::string to_string () const;
  private:
    friend class client;
    struct implementation;

    std::unique_ptr<implementation> _;
  };
}
