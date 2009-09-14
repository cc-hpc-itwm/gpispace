#ifndef  FHG_LOG_LOGMACROS_INC
#define  FHG_LOG_LOGMACROS_INC

#include <fhglog/Logger.hpp>
#include <sstream>

namespace fhg { namespace log {

// TODO: perform some magic to discover whether we have __PRETTY_FUNCTION__ or not
// as it seems it's not macro
#define FHGLOG_FUNCTION __PRETTY_FUNCTION__

#define LOG_TRACE(logger, msg) do { using namespace fhg::log;\
                                    if (logger.isLevelEnabled(LogLevel::TRACE)) { ::std::ostringstream osstr; osstr << msg; logger.log(LogEvent(LogLevel(LogLevel::TRACE), logger.name(), __FILE__, FHGLOG_FUNCTION, __LINE__, osstr.str())); } } while(0)
#define LOG_DEBUG(logger, msg) do { using namespace fhg::log;\
                                    if (logger.isLevelEnabled(LogLevel::DEBUG)) { ::std::stringstream osstr; osstr << msg;  logger.log(LogEvent(LogLevel(LogLevel::DEBUG), logger.name(), __FILE__, FHGLOG_FUNCTION, __LINE__, osstr.str())); } } while(0)
#define LOG_INFO(logger, msg)  do { using namespace fhg::log;\
                                    if (logger.isLevelEnabled(LogLevel::INFO))  { ::std::stringstream osstr; osstr << msg;  logger.log(LogEvent(LogLevel(LogLevel::INFO), logger.name(),  __FILE__, FHGLOG_FUNCTION, __LINE__, osstr.str())); } } while(0)
#define LOG_WARN(logger, msg)  do { using namespace fhg::log;\
                                    if (logger.isLevelEnabled(LogLevel::WARN))  { ::std::ostringstream osstr; osstr << msg; logger.log(LogEvent(LogLevel(LogLevel::WARN), logger.name(),  __FILE__, FHGLOG_FUNCTION, __LINE__, osstr.str())); } } while(0)
#define LOG_ERROR(logger, msg) do { using namespace fhg::log;\
                                   if (logger.isLevelEnabled(LogLevel::ERROR))  { ::std::ostringstream osstr; osstr << msg; logger.log(LogEvent(LogLevel(LogLevel::ERROR), logger.name(), __FILE__, FHGLOG_FUNCTION, __LINE__, osstr.str())); } } while(0)
#define LOG_FATAL(logger, msg) do { using namespace fhg::log;\
                                   if (logger.isLevelEnabled(LogLevel::FATAL))  { ::std::ostringstream osstr; osstr << msg; logger.log(LogEvent(LogLevel(LogLevel::FATAL), logger.name(), __FILE__, FHGLOG_FUNCTION, __LINE__, osstr.str())); } } while(0)
}}

#endif   /* ----- #ifndef FHG_LOG_LOGMACROS_INC  ----- */
