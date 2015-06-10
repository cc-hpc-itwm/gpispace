#pragma once

#include <drts/client.fwd.hpp>
#include <drts/drts.fwd.hpp>
#include <drts/pimpl.hpp>

#include <string>

namespace gspc
{
  class information_to_reattach
  {
  public:
    information_to_reattach (scoped_runtime_system const&);
    information_to_reattach (std::string const&);
    std::string to_string () const;

  private:
    friend class ::gspc::client;

    PIMPL (information_to_reattach);
  };
}
