#ifndef  FHG_LOG_LOGMACROS_INC
#define  FHG_LOG_LOGMACROS_INC

#include <fhglog/LoggerApi.hpp>
#include <fhglog/Configuration.hpp>
#include <sstream>
#include <boost/filesystem.hpp>

namespace fhg { namespace log {

#define FHGLOG_FUNCTION __PRETTY_FUNCTION__

// convenience macro to create an event
#define FHGLOG_MKEVENT_HERE(level, message) ::fhg::log::LogEvent(::fhg::log::LogLevel::level, __FILE__, FHGLOG_FUNCTION, __LINE__, message)
#define FHGLOG_MKEVENT(var, level, message) ::fhg::log::LogEvent var(::fhg::log::LogLevel::level, __FILE__, FHGLOG_FUNCTION, __LINE__, message)

#define FHGLOG_TOSTRING_(x) #x
#define FHGLOG_TOSTRING(x) FHGLOG_TOSTRING_(x)

#define FHGLOG_UNIQUE_NAME(name, line) FHGLOG_UNIQUE_NAME_CONCAT(name, line)
#define FHGLOG_UNIQUE_NAME_CONCAT(name, line) name ## line
#define FHGLOG_COUNTER FHGLOG_UNIQUE_NAME(counter_, __LINE__)
#define FHGLOG_ONCE_FLAG FHGLOG_UNIQUE_NAME(once_, __LINE__)

#  define FHGLOG_FLUSH()                                                \
    do                                                                  \
    {                                                                   \
      fhg::log::getLogger().flush();                                    \
    } while (0)                                                         \

    namespace detail
    {
      struct flush_at_end_of_scope_t
      {
        ~flush_at_end_of_scope_t() throw ()
        {
          FHGLOG_FLUSH();
        }
      };
    }

#  define FHGLOG_SETUP(args...)                                         \
    fhg::log::detail::flush_at_end_of_scope_t fhglog_flush_at_end_of_scope; \
    (void)(fhglog_flush_at_end_of_scope);                               \
    do                                                                  \
    {                                                                   \
      if (! #args[0])                                                   \
      {                                                                 \
        fhg::log::configure();                                          \
      }                                                                 \
      else                                                              \
      {                                                                 \
        fhg::log::configure(args);                                      \
      }                                                                 \
    } while (0)

#  define FHGLOG_TERM()                         \
    do                                          \
    {                                           \
      fhg::log::terminate ();                   \
    } while (0)
#ifndef FHGLOG_STRIP_LEVEL
#  define FHGLOG_STRIP_LEVEL -1
#endif

#  define __LOG(logger, level, msg)                                     \
    do {                                                                \
      using namespace fhg::log;                                         \
      if (  (LogLevel::level > FHGLOG_STRIP_LEVEL)                      \
         && logger.isLevelEnabled(LogLevel::level)                      \
         )                                                              \
      {                                                                 \
        FHGLOG_MKEVENT(__log_evt, level, "");                           \
        if (! logger.isFiltered(__log_evt))                             \
        {                                                               \
          std::ostringstream msg_; msg_ << msg;                         \
          __log_evt.message() = msg_.str();                             \
          logger.log(__log_evt);                                        \
        }                                                               \
      }                                                                 \
    } while(0)

// log to a specific logger
#define LLOG(level, logger, msg) __LOG(logger, level, msg)
#define LLOG_IF(level, logger, condition, msg)                          \
    do                                                                  \
    {                                                                   \
      if (condition)                                                    \
      {                                                                 \
        LLOG(level, logger, msg);                                       \
      }                                                                 \
    }                                                                   \
    while (0)

    // log if some condition is true
#define LOG_IF(level, condition, msg)                                   \
    do                                                                  \
    {                                                                   \
      if (condition)                                                    \
      {                                                                 \
        LOG(level, msg);                                                \
      }                                                                 \
    }                                                                   \
    while (0)

// log to a logger with the name of the filename the statement is in
#define MLOG(level, msg)                                                \
    LLOG( level                                                         \
        , ::fhg::log::getLogger                                         \
          (::boost::filesystem::path (__FILE__).stem().string())        \
        , msg                                                           \
        )

#define MLOG_IF(level, condition, msg)                                  \
    LLOG_IF( level                                                      \
           , ::fhg::log::getLogger                                      \
             (::boost::filesystem::path (__FILE__).stem().string())     \
           , condition                                                  \
           , msg                                                        \
           )

    // log to a named logger (component)
#define CLOG(level, component, msg)                                     \
    LLOG( level                                                         \
        , ::fhg::log::getLogger(component)                              \
        , msg                                                           \
        )
#define CLOG_IF(level, component, condition, msg)                       \
    LLOG_IF( level                                                      \
           , ::fhg::log::getLogger(component)                           \
           , condition                                                  \
           , msg                                                        \
           )

    // just log
#define LOG(level, msg) LLOG(level, ::fhg::log::getLogger(), msg)

#ifdef NDEBUG

#define DMLOG(level, msg)
#define DLOG(level, msg)
#define DLOG_IF(level, condition, msg)

#define DMLOG_IF(level, condition, msg)

#else

#define DMLOG(level, msg) MLOG(level, msg)
#define DLOG(level, msg) LOG(level, msg)
#define DLOG_IF(level, condition, msg) LOG_IF(level, condition, msg)

#define DMLOG_IF(level, condition, msg) MLOG_IF(level, condition, msg)

#endif

}}
#endif   /* ----- #ifndef FHG_LOG_LOGMACROS_INC  ----- */
