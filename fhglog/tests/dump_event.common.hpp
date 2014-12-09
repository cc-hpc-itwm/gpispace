#ifndef FHGLOG_TEST_DUMP_EVENT_COMMON_HPP
#define FHGLOG_TEST_DUMP_EVENT_COMMON_HPP

#include <fhglog/event.hpp>

namespace
{
  fhg::log::LogEvent gen_event()
  {
    std::vector<std::string> tags;
    tags.push_back ("foo");
    tags.push_back ("bar");

    fhg::log::LogEvent evt ( fhg::log::TRACE
                           , __FILE__
                           , "main", __LINE__, "hello world!"
                           , tags
                           );

    evt.trace ("trace1\"trace1");
    evt.trace ("t2't2\"\"\"");

    return evt;
  }
}

#endif
