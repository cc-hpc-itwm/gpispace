#pragma once

#include <fhglog/event.hpp>

namespace
{
  fhg::log::LogEvent gen_event()
  {
    fhg::log::LogEvent evt ( fhg::log::TRACE
                           , __FILE__
                           , "main", __LINE__, "hello world!"
                           );

    evt.trace ("trace1\"trace1");
    evt.trace ("t2't2\"\"\"");

    return evt;
  }
}
