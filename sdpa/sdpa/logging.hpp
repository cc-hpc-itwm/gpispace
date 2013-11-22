#ifndef SDPA_LOGGING_HPP
#define SDPA_LOGGING_HPP 1

#include <sdpa/sdpa-config.hpp>

#include <fhglog/fhglog.hpp>

#define SDPA_LDECLARE_LOGGER(logger)           mutable ::fhg::log::logger_t logger
#define SDPA_LDEFINE_LOGGER(logger, name) ::fhg::log::logger_t logger(::fhg::log::getLogger(name))
#define SDPA_LINIT_LOGGER(logger, name)   logger(::fhg::log::getLogger(name))

#define SDPA_DECLARE_LOGGER()         SDPA_LDECLARE_LOGGER(sdpa_logger)
#define SDPA_DEFINE_LOGGER(hierarchy) SDPA_LDEFINE_LOGGER(sdpa_logger, hierarchy)
#define SDPA_INIT_LOGGER(hierarchy)   SDPA_LINIT_LOGGER(sdpa_logger, hierarchy)

#define SDPA_LLOG_DEBUG(logger, msg) LLOG(DEBUG, logger, msg)
#define SDPA_LLOG_INFO(logger, msg)  LLOG(INFO,  logger, msg)
#define SDPA_LLOG_WARN(logger, msg)  LLOG(WARN,  logger, msg)
#define SDPA_LLOG_ERROR(logger, msg) LLOG(ERROR, logger, msg)
#define SDPA_LLOG_FATAL(logger, msg) LLOG(FATAL, logger, msg)

#define SDPA_LOG_DEBUG(msg) SDPA_LLOG_DEBUG(sdpa_logger, msg)
#define SDPA_LOG_INFO(msg)  SDPA_LLOG_INFO(sdpa_logger, msg)
#define SDPA_LOG_WARN(msg)  SDPA_LLOG_WARN(sdpa_logger, msg)
#define SDPA_LOG_ERROR(msg) SDPA_LLOG_ERROR(sdpa_logger, msg)
#define SDPA_LOG_FATAL(msg) SDPA_LLOG_FATAL(sdpa_logger, msg)

#endif
