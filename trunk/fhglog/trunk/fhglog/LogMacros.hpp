#ifndef  FHG_LOG_LOGMACROS_INC
#define  FHG_LOG_LOGMACROS_INC

#include <fhglog/Logger.hpp>
#include <sstream>

namespace fhg { namespace log {

// TODO: perform some magic to discover whether we have __PRETTY_FUNCTION__ or not
// as it seems it's not macro
#define FHGLOG_FUNCTION __PRETTY_FUNCTION__

// convenience macro to create an event
#define FHGLOG_MKEVENT_HERE(level, message) ::fhg::log::LogEvent(::fhg::log::LogLevel::level, __FILE__, FHGLOG_FUNCTION, __LINE__, message)
#define FHGLOG_MKEVENT(var, level, message) ::fhg::log::LogEvent var(::fhg::log::LogLevel::level, __FILE__, FHGLOG_FUNCTION, __LINE__, message)

#if FHGLOG_DISABLE_LOGGING == 1

#define LOG(logger, level, msg)

#else

#define LOG(logger, level, msg) do { using namespace fhg::log; if (logger.isLevelEnabled(LogLevel::level)) { FHGLOG_MKEVENT(evt, level, ""); if (! logger.isFiltered(evt)) { evt.stream() << msg; logger.log(evt); }}} while(0)
#endif // if FHGLOG_ENABLED == 0

#define LOG_TRACE(logger, msg) LOG(logger, TRACE, msg)
#define LOG_DEBUG(logger, msg) LOG(logger, DEBUG, msg)
#define LOG_INFO(logger, msg)  LOG(logger, INFO, msg)
#define LOG_WARN(logger, msg)  LOG(logger, WARN, msg)
#define LOG_ERROR(logger, msg) LOG(logger, ERROR, msg)
#define LOG_FATAL(logger, msg) LOG(logger, FATAL, msg)

#endif   /* ----- #ifndef FHG_LOG_LOGMACROS_INC  ----- */
}}
