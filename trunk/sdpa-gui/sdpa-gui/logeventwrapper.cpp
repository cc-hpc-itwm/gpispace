#include "logeventwrapper.hpp"
#include <boost/lexical_cast.hpp>

LogEventWrapper::LogEventWrapper(fhg::log::LogEvent const &e)
  : QEvent ((QEvent::Type)1001)
  , log_event (e)
{}

LogEventWrapper::~LogEventWrapper() {}
