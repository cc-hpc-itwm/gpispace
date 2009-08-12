#include	"Appender.hpp"

using namespace fhg::log;


void
ConsoleAppender::append(const LogEvent &evt)
{
  stream_ << fmt_->format(evt);
}		/* -----  end of method ConsoleAppender::append  ----- */
