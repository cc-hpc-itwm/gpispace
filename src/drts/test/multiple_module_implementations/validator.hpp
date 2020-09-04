#pragma once

#include <boost/format.hpp>

#include <stdexcept>
#include <string>

namespace
{
  void validate_implementation
    ( drts::worker::context const* context
    , std::string const& expected_implementation
    )
  {
    auto const workers (context->workers());
    if (workers.size() != 1)
    {
      throw std::runtime_error
        ( ( boost::format ("Unexpected number of workers: %1%!")
          % workers.size()
          ).str()
        );
    }

    auto const worker (context->worker_name());
    if (worker.find (expected_implementation) == std::string::npos)
    {
      throw std::runtime_error
        ( ( boost::format ("Worker %1%: missing capability %2%!")
          % worker
          % expected_implementation
          ).str()
        );
    }
  }
}
