#ifndef SDPA_LOGGING_HPP
#define SDPA_LOGGING_HPP 1

#if HAVE_CONFIG_H
#  include <config.h>
#endif

/* Logging */
#if ENABLE_LOGGING == 1

#if HAVE_LOG4CPP

#include <log4cpp/Category.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/convenience.h>

namespace sdpa {
namespace logging {

typedef ::log4cpp::Priority::PriorityLevel PriorityLevel;

#define SDPA_LDECLARE_LOGGER(logger)           ::log4cpp::Category& logger
#define SDPA_LDEFINE_LOGGER(logger, hierarchy) ::log4cpp::Category& SDPA_LINIT_LOGGER(logger, hierarchy)
#define SDPA_LINIT_LOGGER(logger, hierarchy)   logger(::log4cpp::Category::getInstance(hierarchy))

#define SDPA_DECLARE_LOGGER()         SDPA_LDECLARE_LOGGER(sdpa_logger)
#define SDPA_DEFINE_LOGGER(hierarchy) SDPA_LDEFINE_LOGGER(sdpa_logger, hierarchy)
#define SDPA_INIT_LOGGER(hierarchy)   SDPA_LINIT_LOGGER(sdpa_logger, hierarchy)

#define SDPA_LLOG_DEBUG(logger, msg) LOG4CPP_DEBUG_S(logger) << msg
#define SDPA_LLOG_INFO(logger, msg)  LOG4CPP_INFO_S(logger)  << msg
#define SDPA_LLOG_WARN(logger, msg)  LOG4CPP_WARN_S(logger)  << msg
#define SDPA_LLOG_ERROR(logger, msg) LOG4CPP_ERROR_S(logger) << msg
#define SDPA_LLOG_FATAL(logger, msg) LOG4CPP_FATAL_S(logger) << msg

#define SDPA_LOG_DEBUG(msg) SDPA_LLOG_DEBUG(sdpa_logger, msg)
#define SDPA_LOG_INFO(msg)  SDPA_LLOG_INFO(sdpa_logger, msg) 
#define SDPA_LOG_WARN(msg)  SDPA_LLOG_WARN(sdpa_logger, msg) 
#define SDPA_LOG_ERROR(msg) SDPA_LLOG_ERROR(sdpa_logger, msg)
#define SDPA_LOG_FATAL(msg) SDPA_LLOG_FATAL(sdpa_logger, msg)

#else

#warn Logging has been enabled, but no usable logging mechanism available, disabling it!
#undef ENABLE_LOGGING
#define ENABLE_LOGGING 0

#endif // HAVE_LOG4CPP
#endif // ENABLE_LOGGING == 1

#if ENABLE_LOGGING == 0 /* ! ENABLE_LOGGING */

typedef enum {
    EMERG  = 0,
    FATAL  = 0,
    ALERT  = 100,
    CRIT   = 200,
    ERROR  = 300,
    WARN   = 400,
    NOTICE = 500,
    INFO   = 600,
    DEBUG  = 700,
    NOTSET = 800
} PriorityLevel;

#define SDPA_LDECLARE_LOGGER(logger)   void* __sdpa_unused_##logger
#define SDPA_LDEFINE_LOGGER(logger, h)
#define SDPA_LINIT_LOGGER(logger, h)   __sdpa_unused_##logger(0)

#define SDPA_DECLARE_LOGGER()          SDPA_LDECLARE_LOGGER(logger)
#define SDPA_DEFINE_LOGGER(hierarchy)  SDPA_LDEFINE_LOGGER(logger, hierarchy)
#define SDPA_INIT_LOGGER(hierarchy)    SDPA_LINIT_LOGGER(logger, hierarchy)

#define SDPA_LLOG_DEBUG(logger, msg) 
#define SDPA_LLOG_INFO(logger, msg)  
#define SDPA_LLOG_WARN(logger, msg)  
#define SDPA_LLOG_ERROR(logger, msg) 
#define SDPA_LLOG_FATAL(logger, msg) 

#define SDPA_LOG_DEBUG(msg) 
#define SDPA_LOG_INFO(msg)  
#define SDPA_LOG_WARN(msg)  
#define SDPA_LOG_ERROR(msg) 
#define SDPA_LOG_FATAL(msg) 

#endif
}}

#endif // !SDPA_LOGGING_HPP
