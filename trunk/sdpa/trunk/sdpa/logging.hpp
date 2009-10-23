#ifndef SDPA_LOGGING_HPP
#define SDPA_LOGGING_HPP 1

/* Logging */
#if defined(SDPA_ENABLE_LOGGING)

#if defined(SDPA_HAVE_FHGLOG)

#include <fhglog/fhglog.hpp>

#define SDPA_LDECLARE_LOGGER(logger)           ::fhg::log::logger_t logger
#define SDPA_LDEFINE_LOGGER(logger, name) ::fhg::log::logger_t logger(::fhg::log::getLogger(name))
#define SDPA_LINIT_LOGGER(logger, name)   logger(::fhg::log::getLogger(name))

#define SDPA_DECLARE_LOGGER()         SDPA_LDECLARE_LOGGER(sdpa_logger)
#define SDPA_DEFINE_LOGGER(hierarchy) SDPA_LDEFINE_LOGGER(sdpa_logger, hierarchy)
#define SDPA_INIT_LOGGER(hierarchy)   SDPA_LINIT_LOGGER(sdpa_logger, hierarchy)

#define SDPA_LLOG_DEBUG(logger, msg) MLOG(DEBUG, msg)
#define SDPA_LLOG_INFO(logger, msg)  MLOG(INFO,  msg)
#define SDPA_LLOG_WARN(logger, msg)  MLOG(WARN,  msg)
#define SDPA_LLOG_ERROR(logger, msg) MLOG(ERROR, msg)
#define SDPA_LLOG_FATAL(logger, msg) MLOG(FATAL, msg)

#define SDPA_LOG_DEBUG(msg) SDPA_LLOG_DEBUG(sdpa_logger, msg)
#define SDPA_LOG_INFO(msg)  SDPA_LLOG_INFO(sdpa_logger, msg) 
#define SDPA_LOG_WARN(msg)  SDPA_LLOG_WARN(sdpa_logger, msg) 
#define SDPA_LOG_ERROR(msg) SDPA_LLOG_ERROR(sdpa_logger, msg)
#define SDPA_LOG_FATAL(msg) SDPA_LLOG_FATAL(sdpa_logger, msg)

#elif defined(SDPA_HAVE_LOG4CPP)

#include <log4cpp/Category.hh>
#include <log4cpp/convenience.h>

#define SDPA_LDECLARE_LOGGER(logger)           ::log4cpp::Category& logger
#define SDPA_LDEFINE_LOGGER(logger, hierarchy) ::log4cpp::Category& SDPA_LINIT_LOGGER(logger, hierarchy)
#define SDPA_LINIT_LOGGER(logger, hierarchy)   logger(::log4cpp::Category::getInstance(hierarchy))

#define SDPA_DECLARE_LOGGER()         SDPA_LDECLARE_LOGGER(seda_logger)
#define SDPA_DEFINE_LOGGER(hierarchy) SDPA_LDEFINE_LOGGER(seda_logger, hierarchy)
#define SDPA_INIT_LOGGER(hierarchy)   SDPA_LINIT_LOGGER(seda_logger, hierarchy)

#define SDPA_LLOG_DEBUG(logger, msg) LOG4CPP_DEBUG_S(logger) << msg
#define SDPA_LLOG_INFO(logger, msg)  LOG4CPP_INFO_S(logger)  << msg
#define SDPA_LLOG_WARN(logger, msg)  LOG4CPP_WARN_S(logger)  << msg
#define SDPA_LLOG_ERROR(logger, msg) LOG4CPP_ERROR_S(logger) << msg
#define SDPA_LLOG_FATAL(logger, msg) LOG4CPP_FATAL_S(logger) << msg

#define SDPA_LOG_DEBUG(msg) SDPA_LLOG_DEBUG(seda_logger, msg)
#define SDPA_LOG_INFO(msg)  SDPA_LLOG_INFO(seda_logger, msg) 
#define SDPA_LOG_WARN(msg)  SDPA_LLOG_WARN(seda_logger, msg) 
#define SDPA_LOG_ERROR(msg) SDPA_LLOG_ERROR(seda_logger, msg)
#define SDPA_LOG_FATAL(msg) SDPA_LLOG_FATAL(seda_logger, msg)

#else
#undef SDPA_ENABLE_LOGGING
//#warning Logging has been enabled, but no usable logging mechanism available, disabling it!

#endif // HAVE_FHGLOG
#endif // ENABLE_LOGGING == 1

#if not defined (SDPA_ENABLE_LOGGING)

#define SDPA_LDECLARE_LOGGER(logger)   void* __seda_unused_##logger
#define SDPA_LDEFINE_LOGGER(logger, h)
#define SDPA_LINIT_LOGGER(logger, h)   __seda_unused_##logger(0)

#define SDPA_DECLARE_LOGGER()          SDPA_LDECLARE_LOGGER(logger)
#define SDPA_DEFINE_LOGGER(hierarchy)  SDPA_LDEFINE_LOGGER(logger, hierarchy)
#define SDPA_INIT_LOGGER(hierarchy)    SDPA_LINIT_LOGGER(logger, hierarchy)

#define SDPA_LLOG_DEBUG(logger, msg) 
#define SDPA_LLOG_INFO(logger, msg)  
#define SDPA_LLOG_WARN(logger, msg)  
#define SDPA_LLOG_ERROR(logger, msg) 
#define SDPA_LLOG_FATAL(logger, msg) 

#define SDPA_LOG_DEBUG(msg)  do {} while(0)
#define SDPA_LOG_INFO(msg)   do {} while(0)
#define SDPA_LOG_WARN(msg)   do {} while(0)
#define SDPA_LOG_ERROR(msg)  do {} while(0)
#define SDPA_LOG_FATAL(msg)  do {} while(0)

#endif

#endif // !SDPA_LOGGING_HPP
