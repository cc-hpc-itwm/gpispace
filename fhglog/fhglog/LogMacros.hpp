#ifndef FHG_LOG_LOGMACROS_INC
#define FHG_LOG_LOGMACROS_INC

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

#define LLOG(level, logger, msg)                                \
    do                                                          \
    {                                                           \
      if (logger->isLevelEnabled (::fhg::log::level))           \
      {                                                         \
        std::ostringstream msg_;                                \
        msg_ << msg;                                            \
        logger->log (FHGLOG_MKEVENT_HERE (level, msg_.str()));  \
      }                                                         \
    } while (0)

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
