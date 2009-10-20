#ifndef  FHG_LOG_LOGMACROS_INC
#define  FHG_LOG_LOGMACROS_INC

#include <fhglog/Logger.hpp>
#include <fhglog/util.hpp>
#include <sstream>

namespace fhg { namespace log {

// TODO: perform some magic to discover whether we have __PRETTY_FUNCTION__ or not
// as it seems it's not macro
#define FHGLOG_FUNCTION __PRETTY_FUNCTION__

// convenience macro to create an event
#define FHGLOG_MKEVENT_HERE(level, message) ::fhg::log::LogEvent(::fhg::log::LogLevel::level, __FILE__, FHGLOG_FUNCTION, __LINE__, message)
#define FHGLOG_MKEVENT(var, level, message) ::fhg::log::LogEvent var(::fhg::log::LogLevel::level, __FILE__, FHGLOG_FUNCTION, __LINE__, message)

#if FHGLOG_DISABLE_LOGGING == 1
#define __LOG(logger, level, msg)
#else
#define __LOG(logger, level, msg) do { using namespace fhg::log; if (logger.isLevelEnabled(LogLevel::level)) { FHGLOG_MKEVENT(__log_evt, level, ""); if (! logger.isFiltered(__log_evt)) { __log_evt.stream() << msg; logger.log(__log_evt); }}} while(0)
#endif // if FHGLOG_ENABLED == 0

#ifndef NDEBUG
// support for log output only when: logging activated at all and NDEBUG is not defined
// this can be useful to introduce extra log output for debugging purposes but not in releases
#define __DLOG(logger, level, msg) __LOG(logger, level, msg)
#else
#define __DLOG(logger, level, msg)
#endif

#define LLOG(level, logger, msg) __LOG(logger, level, msg)
#define MLOG(level, msg) LLOG(level, ::fhg::log::getLogger(::fhg::log::get_module_name_from_path(__FILE__)), msg)
#define LOG(level, msg) LLOG(level, ::fhg::log::getLogger(), msg)

#define DLLOG(level, logger, msg) __DLOG(logger, level, msg)
#define DMLOG(level, msg) DLLOG(level, ::fhg::log::getLogger(::fhg::log::get_module_name_from_path(__FILE__)), msg)
#define DLOG(level, msg) DLLOG(level, ::fhg::log::getLogger(), msg)

// regular logging messages
#define LOG_TRACE(logger, msg) __LOG(logger, TRACE, msg)
#define LOG_DEBUG(logger, msg) __LOG(logger, DEBUG, msg)
#define LOG_INFO(logger, msg)  __LOG(logger, INFO, msg)
#define LOG_WARN(logger, msg)  __LOG(logger, WARN, msg)
#define LOG_ERROR(logger, msg) __LOG(logger, ERROR, msg)
#define LOG_FATAL(logger, msg) __LOG(logger, FATAL, msg)

// only if NDEBUG has not been defined (i.e. in release builds)
#define DLOG_TRACE(logger, msg) __DLOG(logger, TRACE, msg)
#define DLOG_DEBUG(logger, msg) __DLOG(logger, DEBUG, msg)
#define DLOG_INFO(logger, msg)  __DLOG(logger, INFO, msg)
#define DLOG_WARN(logger, msg)  __DLOG(logger, WARN, msg)
#define DLOG_ERROR(logger, msg) __DLOG(logger, ERROR, msg)
#define DLOG_FATAL(logger, msg) __DLOG(logger, FATAL, msg)

#endif   /* ----- #ifndef FHG_LOG_LOGMACROS_INC  ----- */
}}
