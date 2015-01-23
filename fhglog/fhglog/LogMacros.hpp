#ifndef FHG_LOG_LOGMACROS_INC
#define FHG_LOG_LOGMACROS_INC

#include <fhglog/Configuration.hpp>
#include <fhglog/Logger.hpp>

#include <boost/current_function.hpp>
#include <boost/filesystem.hpp>

#include <sstream>

namespace fhg
{
  namespace log
  {

#define FHGLOG_MKEVENT_HERE(level, message)             \
    ::fhg::log::LogEvent ( ::fhg::log::level            \
                         , __FILE__                     \
                         , BOOST_CURRENT_FUNCTION       \
                         , __LINE__                     \
                         , message                      \
                         )

    struct flush_at_end_of_scope_t
    {
      ~flush_at_end_of_scope_t()
      {
        ::fhg::log::Logger::get()->flush();
      }
    };

#define FHGLOG_SETUP(remote_log_io_service)                             \
    ::fhg::log::flush_at_end_of_scope_t _1e872d1a_6dcf_11e3_9e3b_13f239ecd3ca; \
    ::fhg::log::configure (remote_log_io_service)

#define __LOG(logger, level, msg)                               \
    do                                                          \
    {                                                           \
      if (logger->isLevelEnabled (::fhg::log::level))           \
      {                                                         \
        std::ostringstream msg_;                                \
        msg_ << msg;                                            \
        logger->log (FHGLOG_MKEVENT_HERE (level, msg_.str()));  \
      }                                                         \
    } while (0)

#define LLOG(level, logger, msg) __LOG(logger, level, msg)
#define LLOG_IF(level, logger, condition, msg)  \
    do                                          \
    {                                           \
      if (condition)                            \
      {                                         \
        LLOG (level, logger, msg);              \
      }                                         \
    }                                           \
    while (0)

    // log if some condition is true
#define LOG_IF(level, condition, msg)           \
    do                                          \
    {                                           \
      if (condition)                            \
      {                                         \
        LOG (level, msg);                       \
      }                                         \
    }                                           \
    while (0)

    // log to a logger with the name of the filename the statement is in
#define MLOG(level, msg)                                          \
    LLOG ( level                                                  \
         , ::fhg::log::Logger::get                                \
           (::boost::filesystem::path (__FILE__).stem().string()) \
         , msg                                                    \
         )

#define MLOG_IF(level, condition, msg)                                    \
    LLOG_IF ( level                                                       \
            , ::fhg::log::Logger::get                                     \
              (::boost::filesystem::path (__FILE__).stem().string())      \
            , condition                                                   \
            , msg                                                         \
            )

    // log to a named logger (component)
#define CLOG(level, component, msg)             \
    LLOG ( level                                \
         , ::fhg::log::Logger::get (component)    \
         , msg                                  \
         )

    // just log
#define LOG(level, msg) LLOG (level, ::fhg::log::Logger::get(), msg)
  }
}

#endif
