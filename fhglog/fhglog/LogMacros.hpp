#pragma once

#include <fhglog/Logger.hpp>

#include <boost/current_function.hpp>
#include <boost/filesystem.hpp>

#include <sstream>

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
      if (logger.isLevelEnabled (::fhg::log::level))            \
      {                                                         \
        std::ostringstream msg_;                                \
        msg_ << msg;                                            \
        logger.log (FHGLOG_MKEVENT_HERE (level, msg_.str()));   \
      }                                                         \
    } while (0)
