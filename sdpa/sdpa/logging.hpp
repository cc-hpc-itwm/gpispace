#ifndef SDPA_LOGGING_HPP
#define SDPA_LOGGING_HPP 1

#include <fhglog/fhglog.hpp>

#define SDPA_DECLARE_LOGGER() mutable ::fhg::log::Logger::ptr_t sdpa_logger
#define SDPA_INIT_LOGGER(name) sdpa_logger (::fhg::log::Logger::get (name))

#define SDPA_LOG_DEBUG(msg) LLOG(DEBUG, sdpa_logger, msg)
#define SDPA_LOG_INFO(msg) LLOG(INFO, sdpa_logger, msg)
#define SDPA_LOG_WARN(msg) LLOG(WARN, sdpa_logger, msg)
#define SDPA_LOG_ERROR(msg) LLOG(ERROR, sdpa_logger, msg)
#define SDPA_LOG_FATAL(msg) LLOG(FATAL, sdpa_logger, msg)

#endif
