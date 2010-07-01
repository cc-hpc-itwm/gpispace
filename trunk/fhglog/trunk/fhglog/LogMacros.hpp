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

#define FHGLOG_TOSTRING_(x) #x
#define FHGLOG_TOSTRING(x) FHGLOG_TOSTRING_(x)

#define FHGLOG_UNIQUE_NAME(name, line) FHGLOG_UNIQUE_NAME_CONCAT(name, line)
#define FHGLOG_UNIQUE_NAME_CONCAT(name, line) name ## line
#define FHGLOG_COUNTER FHGLOG_UNIQUE_NAME(counter_, __LINE__)

#define FHGLOG_DO_EVERY_N(n, thing)                                     \
  do {                                                                  \
    static unsigned int FHGLOG_COUNTER = 0;                             \
    if ((FHGLOG_COUNTER++ % n) == 0)                                    \
    {                                                                   \
      thing;                                                            \
    }                                                                   \
  } while (0)

#define FHGLOG_DO_EVERY_N_IF(condition, n, thing)                       \
  do {                                                                  \
    if (condition)                                                      \
    {                                                                   \
      static unsigned int FHGLOG_COUNTER = 0;                           \
      if ((FHGLOG_COUNTER++ % n) == 0)                                  \
      {                                                                 \
        thing;                                                          \
      }                                                                 \
    }                                                                   \
  } while (0)

#if FHGLOG_DISABLE_LOGGING == 1
#define __LOG(logger, level, msg)
#else
#define __LOG(logger, level, msg)                                       \
    do {                                                                \
      using namespace fhg::log;                                         \
      if (logger.isLevelEnabled(LogLevel::level))                       \
      {                                                                 \
        FHGLOG_MKEVENT(__log_evt, level, "");                           \
        if (! logger.isFiltered(__log_evt))                             \
        {                                                               \
          __log_evt.stream() << msg;                                    \
          logger.log(__log_evt);                                        \
        }                                                               \
      }                                                                 \
    } while(0)
#endif // if FHGLOG_ENABLED == 1

#define LLOG(level, logger, msg) __LOG(logger, level, msg)
#define MLOG(level, msg) LLOG(level, ::fhg::log::getLogger(::fhg::log::get_module_name_from_path(__FILE__)), msg)
#define LOG(level, msg) LLOG(level, ::fhg::log::getLogger(), msg)
#define LOG_IF(level, condition, msg)                                   \
    do                                                                  \
    {                                                                   \
      if (condition)                                                    \
      {                                                                 \
        LOG(level, msg);                                                \
      }                                                                 \
    }                                                                   \
    while (0)
#define LOG_IF_ELSE(level, condition, then_msg, else_msg)               \
    do                                                                  \
    {                                                                   \
      if (condition)                                                    \
      {                                                                 \
        LOG(level, then_msg);                                           \
      }                                                                 \
      else                                                              \
      {                                                                 \
        LOG(level, else_msg);                                           \
      }                                                                 \
    }                                                                   \
    while (0)

#define LOG_EVERY_N(level, N, msg) FHGLOG_DO_EVERY_N(N, LOG(level, msg))
#define LOG_EVERY_N_IF(level, N, condition, msg) FHGLOG_DO_EVERY_N_IF(condition, N, LOG(level, msg))

#ifdef NDEBUG

#define DLLOG(level, logger, msg)
#define DMLOG(level, msg)
#define DLOG(level, msg)
#define DLOG_IF(level, condition, msg)
#define DLOG_IF_ELSE(level, condition, m1, m2)
#define DLOG_EVERY_N(level, N, msg)
#define DLOG_EVERY_N_IF(level, N, condition, msg)

#else

#define DLLOG(level, logger, msg) LLOG(level, logger, msg)
#define DMLOG(level, msg) MLOG(level, msg)
#define DLOG(level, msg) LOG(level, msg)
#define DLOG_IF(level, condition, msg) LOG_IF(level, condition, msg)
#define DLOG_IF_ELSE(level, condition, msg1, msg2) LOG_IF_ELSE(level, condition, msg1, msg2)
#define DLOG_EVERY_N(level, N, msg) LOG_EVERY_N(level, N, msg)
#define DLOG_EVERY_N_IF(level, N, condition, msg) LOG_EVERY_N_IF(level, N, condition, msg)

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
