#include "logeventwrapper.hpp"
#include <boost/lexical_cast.hpp>

LogEventWrapper::LogEventWrapper(int typ, fhg::log::LogEvent const &e)
  : QEvent ((QEvent::Type)typ)
  , log_event (e)
{}

LogEventWrapper::~LogEventWrapper() {}
