#pragma once

#include <fhglog/event.hpp>

namespace
{
  fhg::log::LogEvent gen_event()
  {
    return {fhg::log::TRACE, "hello world!"};
  }
}
