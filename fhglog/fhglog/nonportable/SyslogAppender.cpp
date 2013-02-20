// alexander.petry@itwm.fraunhofer.de

#include "SyslogAppender.hpp"
#include <fhglog/format.hpp>
using namespace fhg::log;

void SyslogAppender::append (const fhg::log::LogEvent& evt)
{
  syslog(evt.severity().lvl(), "%s", format(_fmt, evt).c_str());
}
