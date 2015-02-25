#pragma once

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

    // just log
#define LOG(level, msg) LLOG (level, ::fhg::log::GLOBAL_logger(), msg)
  }
}
