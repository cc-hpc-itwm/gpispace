#ifndef  FHG_LOG_LOGMACROS_INC
#define  FHG_LOG_LOGMACROS_INC

#include <fhglog/LoggerApi.hpp>
#include <fhglog/Configuration.hpp>
#include <sstream>
#include <boost/filesystem.hpp>

namespace fhg { namespace log {

// convenience macro to create an event
#define FHGLOG_MKEVENT_HERE(level, message) ::fhg::log::LogEvent(::fhg::log::LogLevel::level, __FILE__, __PRETTY_FUNCTION__, __LINE__, message)
#define FHGLOG_MKEVENT(var, level, message) ::fhg::log::LogEvent var(::fhg::log::LogLevel::level, __FILE__, __PRETTY_FUNCTION__, __LINE__, message)

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

#  define FHGLOG_SETUP()                                                \
    fhg::log::detail::flush_at_end_of_scope_t fhglog_flush_at_end_of_scope; \
    (void)(fhglog_flush_at_end_of_scope);                               \
    fhg::log::configure();

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
        std::ostringstream msg_;                                        \
        msg_ << msg;                                                    \
        FHGLOG_MKEVENT(__log_evt, level, msg_.str());                   \
                                                                        \
        if (! logger.isFiltered(__log_evt))                             \
        {                                                               \
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
