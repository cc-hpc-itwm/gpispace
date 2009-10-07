#ifndef SEDA_LOGGING_HPP
#define SEDA_LOGGING_HPP 1

/* Logging */
#if SEDA_ENABLE_LOGGING == 1

#if SEDA_HAVE_LOG4CXX
#include <log4cxx/logger.h>

#define SEDA_LDECLARE_LOGGER(logger)           ::log4cxx::LoggerPtr logger
#define SEDA_LDEFINE_LOGGER(logger, hierarchy) ::log4cxx::LoggerPtr SEDA_LINIT_LOGGER(logger, hierarchy)
#define SEDA_LINIT_LOGGER(logger, hierarchy)   logger(::log4cxx::Logger::getLogger(hierarchy))

#define SEDA_DECLARE_LOGGER()         SEDA_LDECLARE_LOGGER(seda_logger)
#define SEDA_DEFINE_LOGGER(hierarchy) SEDA_LDEFINE_LOGGER(seda_logger, hierarchy)
#define SEDA_INIT_LOGGER(hierarchy)   SEDA_LINIT_LOGGER(seda_logger, hierarchy)

#define SEDA_LLOG_DEBUG(logger, msg) LOG4CXX_DEBUG(logger, msg)
#define SEDA_LLOG_INFO(logger, msg)  LOG4CXX_INFO(logger, msg)
#define SEDA_LLOG_WARN(logger, msg)  LOG4CXX_WARN(logger, msg)
#define SEDA_LLOG_ERROR(logger, msg) LOG4CXX_ERROR(logger, msg)
#define SEDA_LLOG_FATAL(logger, msg) LOG4CXX_FATAL(logger, msg)

#define SEDA_LOG_DEBUG(msg) SEDA_LLOG_DEBUG(seda_logger, msg)
#define SEDA_LOG_INFO(msg)  SEDA_LLOG_INFO(seda_logger, msg) 
#define SEDA_LOG_WARN(msg)  SEDA_LLOG_WARN(seda_logger, msg) 
#define SEDA_LOG_ERROR(msg) SEDA_LLOG_ERROR(seda_logger, msg)
#define SEDA_LOG_FATAL(msg) SEDA_LLOG_FATAL(seda_logger, msg)

#elif SEDA_HAVE_LOG4CPP

#include <log4cpp/Category.hh>
#include <log4cpp/convenience.h>

#define SEDA_LDECLARE_LOGGER(logger)           ::log4cpp::Category& logger
#define SEDA_LDEFINE_LOGGER(logger, hierarchy) ::log4cpp::Category& SEDA_LINIT_LOGGER(logger, hierarchy)
#define SEDA_LINIT_LOGGER(logger, hierarchy)   logger(::log4cpp::Category::getInstance(hierarchy))

#define SEDA_DECLARE_LOGGER()         SEDA_LDECLARE_LOGGER(seda_logger)
#define SEDA_DEFINE_LOGGER(hierarchy) SEDA_LDEFINE_LOGGER(seda_logger, hierarchy)
#define SEDA_INIT_LOGGER(hierarchy)   SEDA_LINIT_LOGGER(seda_logger, hierarchy)

#define SEDA_LLOG_DEBUG(logger, msg) LOG4CPP_DEBUG_S(logger) << msg
#define SEDA_LLOG_INFO(logger, msg)  LOG4CPP_INFO_S(logger)  << msg
#define SEDA_LLOG_WARN(logger, msg)  LOG4CPP_WARN_S(logger)  << msg
#define SEDA_LLOG_ERROR(logger, msg) LOG4CPP_ERROR_S(logger) << msg
#define SEDA_LLOG_FATAL(logger, msg) LOG4CPP_FATAL_S(logger) << msg

#define SEDA_LOG_DEBUG(msg) SEDA_LLOG_DEBUG(seda_logger, msg)
#define SEDA_LOG_INFO(msg)  SEDA_LLOG_INFO(seda_logger, msg) 
#define SEDA_LOG_WARN(msg)  SEDA_LLOG_WARN(seda_logger, msg) 
#define SEDA_LOG_ERROR(msg) SEDA_LLOG_ERROR(seda_logger, msg)
#define SEDA_LOG_FATAL(msg) SEDA_LLOG_FATAL(seda_logger, msg)

#else

#warn Logging has been enabled, but no usable logging mechanism available, disabling it!
#undef SEDA_ENABLE_LOGGING
#define SEDA_ENABLE_LOGGING 0

#endif // SEDA_HAVE_LOG4CPP
#endif // SEDA_ENABLE_LOGGING == 1

#if SEDA_ENABLE_LOGGING == 0

#define SEDA_LDECLARE_LOGGER(logger)   void* __seda_unused_##logger
#define SEDA_LDEFINE_LOGGER(logger, h)
#define SEDA_LINIT_LOGGER(logger, h)   __seda_unused_##logger(0)

#define SEDA_DECLARE_LOGGER()          SEDA_LDECLARE_LOGGER(logger)
#define SEDA_DEFINE_LOGGER(hierarchy)  SEDA_LDEFINE_LOGGER(logger, hierarchy)
#define SEDA_INIT_LOGGER(hierarchy)    SEDA_LINIT_LOGGER(logger, hierarchy)

#define SEDA_LLOG_DEBUG(logger, msg) 
#define SEDA_LLOG_INFO(logger, msg)  
#define SEDA_LLOG_WARN(logger, msg)  
#define SEDA_LLOG_ERROR(logger, msg) 
#define SEDA_LLOG_FATAL(logger, msg) 

#define SEDA_LOG_DEBUG(msg) 
#define SEDA_LOG_INFO(msg)  
#define SEDA_LOG_WARN(msg)  
#define SEDA_LOG_ERROR(msg) 
#define SEDA_LOG_FATAL(msg) 

#endif

#endif // !SEDA_LOGGING_HPP
