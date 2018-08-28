#pragma once

#include <util-generic/ostream/redirect.hpp>

#include <ostream>
#include <string>

namespace fhg
{
  namespace logging
  {
    struct ostream_redirect : public util::ostream::redirect
    {
      template<typename Emitter>
        ostream_redirect (std::ostream&, Emitter&, std::string category);
    };
  }
}

#include <logging/ostream_redirect.ipp>
