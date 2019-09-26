#pragma once

#include <logging/endpoint.hpp>
#include <logging/stream_receiver.hpp>

#include <vector>

namespace fhg
{
  namespace logging
  {
    struct stdout_sink : private stream_receiver
    {
    public:
      //! \todo Formatter.
      stdout_sink();
      stdout_sink (std::vector<endpoint> emitters);

      using stream_receiver::add_emitters;
    };
  }
}
