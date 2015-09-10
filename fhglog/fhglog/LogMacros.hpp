#pragma once

#include <fhglog/Logger.hpp>

#include <sstream>

#define FHGLOG_MKEVENT_HERE(level, message)             \
    ::fhg::log::LogEvent ( ::fhg::log::level            \
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
