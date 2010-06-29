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
#define __LOG(logger, level, msg)\
    do {\
      using namespace fhg::log;\
      if (logger.isLevelEnabled(LogLevel::level)) \
      {\
        FHGLOG_MKEVENT(__log_evt, level, "");\
        if (! logger.isFiltered(__log_evt))\
        {\
          __log_evt.stream() << msg;\
          logger.log(__log_evt);\
        }\
      }\
    } while(0)
#endif // if FHGLOG_ENABLED == 0

#define LLOG(level, logger, msg) __LOG(logger, level, msg)
#define MLOG(level, msg) LLOG(level, ::fhg::log::getLogger(::fhg::log::get_module_name_from_path(__FILE__)), msg)
#define LOG(level, msg) LLOG(level, ::fhg::log::getLogger(), msg)
#define LOG_IF(level, condition, msg) do { if ((condition)) { LOG(level, msg); } while (0)

#ifndef NDEBUG

#define DLLOG(level, logger, msg) __LOG(logger, level, msg)
#define DMLOG(level, msg) MLOG(level, msg)
#define DLOG(level, msg) LOG(level, msg)
#define DLOG_IF(level, condition, msg) LOG_IF(level, condition, msg)

#else

#define DLLOG(level, logger, msg)
#define DMLOG(level, msg)
#define DLOG(level, msg)
#define DLOG_IF(level, condition, msg)

#endif

// regular logging messages
#define LOG_TRACE(logger, msg) LLOG(TRACE, logger, msg)
#define LOG_DEBUG(logger, msg) LLOG(DEBUG, logger, msg)
#define LOG_INFO(logger, msg)  LLOG(INFO,  logger, msg)
#define LOG_WARN(logger, msg)  LLOG(WARN,  logger, msg)
#define LOG_ERROR(logger, msg) LLOG(ERROR, logger, msg)
#define LOG_FATAL(logger, msg) LLOG(FATAL, logger, msg)

#endif   /* ----- #ifndef FHG_LOG_LOGMACROS_INC  ----- */
}}
