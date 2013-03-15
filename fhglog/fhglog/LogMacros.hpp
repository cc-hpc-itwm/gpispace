#ifndef  FHG_LOG_LOGMACROS_INC
#define  FHG_LOG_LOGMACROS_INC

#if not defined(FHGLOG_DISABLE_LOGGING) || FHGLOG_DISABLE_LOGGING == 0
#  include <fhglog/util.hpp>
#  include <fhglog/LoggerApi.hpp>
#  include <fhglog/Configuration.hpp>
#  include <sstream>
#endif

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
#define FHGLOG_ONCE_FLAG FHGLOG_UNIQUE_NAME(once_, __LINE__)

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

#define FHGLOG_DO_ONCE(thing)                   \
    do {                                        \
      static unsigned int FHGLOG_ONCE_FLAG = 0; \
      if (FHGLOG_ONCE_FLAG == 0)                \
      {                                         \
        thing;                                  \
        FHGLOG_ONCE_FLAG = 1;                   \
      }                                         \
    } while (0)

#define FHGLOG_DO_ONCE_IF(condition, thing)      \
    do {                                         \
      static unsigned int FHGLOG_ONCE_FLAG = 0;  \
      if (FHGLOG_ONCE_FLAG == 0 && (condition))  \
      {                                          \
        thing;                                   \
        FHGLOG_ONCE_FLAG = 1;                    \
      }                                          \
    } while (0)

#if FHGLOG_DISABLE_LOGGING == 1
#  define __LOG(logger, level, msg)
#  define FHGLOG_SETUP(args...)
#  define FHGLOG_FLUSH()
#  define FHGLOG_TERM()
#else
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
    fhg::log::detail::flush_at_end_of_scope_t fhglog_flush_at_end_of_scope;       \
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
          __log_evt.stream() << msg;                                    \
          logger.log(__log_evt);                                        \
        }                                                               \
      }                                                                 \
    } while(0)

#endif // if FHGLOG_ENABLED == 1

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

#define LLOG_IF_ELSE(level, logger, condition, msg_if, msg_else)        \
    do                                                                  \
    {                                                                   \
      if (condition)                                                    \
      {                                                                 \
        LLOG(level, logger, if_msg);                                    \
      }                                                                 \
      else                                                              \
      {                                                                 \
        LLOG(level, logger, else_msg);                                  \
      }                                                                 \
    }                                                                   \
    while (0)

#define LLOG_EVERY_N(level, logger, N, msg) FHGLOG_DO_EVERY_N(N, LLOG(level, logger, msg))
#define LLOG_EVERY_N_IF(level, logger, N, condition, msg) FHGLOG_DO_EVERY_N_IF(condition, N, LLOG(level, logger, msg))

#define LLOG_ONCE(level, logger, msg) FHGLOG_DO_ONCE(LLOG(level, logger, msg))
#define LLOG_ONCE_IF(level, logger, condition, msg) FHGLOG_DO_ONCE_IF(condition, LLOG(level, logger, msg))

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

    // log something if a condition is true, something else otherwise
#define LOG_IF_ELSE(level, condition, if_msg, else_msg)                 \
    do                                                                  \
    {                                                                   \
      if (condition)                                                    \
      {                                                                 \
        LOG(level, if_msg);                                             \
      }                                                                 \
      else                                                              \
      {                                                                 \
        LOG(level, else_msg);                                           \
      }                                                                 \
    }                                                                   \
    while (0)

    // log only every Nth message
#define LOG_EVERY_N(level, N, msg) FHGLOG_DO_EVERY_N(N, LOG(level, msg))
#define LOG_EVERY_N_IF(level, N, condition, msg) FHGLOG_DO_EVERY_N_IF(condition, N, LOG(level, msg))

#define LOG_ONCE(level, msg) FHGLOG_DO_ONCE(LOG(level, msg))
#define LOG_ONCE_IF(level, condition, msg) FHGLOG_DO_ONCE_IF(condition, LOG(level, msg))

// log to a logger with the name of the filename the statement is in
#define MLOG(level, msg)                                                \
    LLOG( level                                                         \
        , ::fhg::log::getLogger                                         \
        (::fhg::log::get_module_name_from_path(__FILE__))               \
        , msg                                                           \
        )

#define MLOG_IF(level, condition, msg)                                  \
    LLOG_IF( level                                                      \
           , ::fhg::log::getLogger                                      \
             (::fhg::log::get_module_name_from_path(__FILE__))          \
           , condition                                                  \
           , msg                                                        \
           )

#define MLOG_EVERY_N(level, N, msg) FHGLOG_DO_EVERY_N(N, MLOG(level, msg))
#define MLOG_EVERY_N_IF(level, N, condition, msg) FHGLOG_DO_EVERY_N_IF(condition, N, MLOG(level, msg))

#define MLOG_ONCE(level, msg) FHGLOG_DO_ONCE(MLOG(level, msg))
#define MLOG_ONCE_IF(level, N, condition, msg) FHGLOG_DO_ONCE_IF(condition, MLOG(level, msg))

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

#define DLLOG(level, logger, msg)
#define DMLOG(level, msg)
#define DCLOG(level, component, msg)
#define DLOG(level, msg)
#define DLOG_IF(level, condition, msg)
#define DLOG_IF_ELSE(level, condition, m1, m2)
#define DLOG_EVERY_N(level, N, msg)
#define DLOG_EVERY_N_IF(level, N, condition, msg)
#define DLOG_ONCE(level, msg)
#define DLOG_ONCE_IF(level, condition, msg)

#define DMLOG_IF(level, condition, msg)
#define DMLOG_IF_ELSE(level, condition, m1, m2)
#define DMLOG_EVERY_N(level, N, msg)
#define DMLOG_EVERY_N_IF(level, N, condition, msg)
#define DMLOG_ONCE(level, msg)
#define DMLOG_ONCE_IF(level, condition, msg)

#else

#define DLLOG(level, logger, msg) LLOG(level, logger, msg)
#define DMLOG(level, msg) MLOG(level, msg)
#define DCLOG(level, component, msg) CLOG(level, component, msg)
#define DLOG(level, msg) LOG(level, msg)
#define DLOG_IF(level, condition, msg) LOG_IF(level, condition, msg)
#define DLOG_IF_ELSE(level, condition, msg1, msg2) LOG_IF_ELSE(level, condition, msg1, msg2)
#define DLOG_EVERY_N(level, N, msg) LOG_EVERY_N(level, N, msg)
#define DLOG_EVERY_N_IF(level, N, condition, msg) LOG_EVERY_N_IF(level, N, condition, msg)
#define DLOG_ONCE(level, msg) LOG_ONCE(level, msg)
#define DLOG_ONCE_IF(level, condition, msg) LOG_ONCE_IF(level, condition, msg)

#define DMLOG_IF(level, condition, msg) MLOG_IF(level, condition, msg)
#define DMLOG_IF_ELSE(level, condition, msg1, msg2) MLOG_IF_ELSE(level, condition, msg1, msg2)
#define DMLOG_EVERY_N(level, N, msg) MLOG_EVERY_N(level, N, msg)
#define DMLOG_EVERY_N_IF(level, N, condition, msg) MLOG_EVERY_N_IF(level, N, condition, msg)
#define DMLOG_ONCE(level, msg) MLOG_ONCE(level, msg)
#define DMLOG_ONCE_IF(level, condition, msg) MLOG_ONCE_IF(level, condition, msg)

#endif

// regular logging messages
#define LOG_TRACE(logger, msg) LLOG(TRACE, logger, msg)
#define LOG_DEBUG(logger, msg) LLOG(DEBUG, logger, msg)
#define LOG_INFO(logger, msg)  LLOG(INFO,  logger, msg)
#define LOG_WARN(logger, msg)  LLOG(WARN,  logger, msg)
#define LOG_ERROR(logger, msg) LLOG(ERROR, logger, msg)
#define LOG_FATAL(logger, msg) LLOG(FATAL, logger, msg)

}}
#endif   /* ----- #ifndef FHG_LOG_LOGMACROS_INC  ----- */
