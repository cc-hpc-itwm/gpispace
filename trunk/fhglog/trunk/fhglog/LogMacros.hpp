#ifndef  FHG_LOG_LOGMACROS_INC
#define  FHG_LOG_LOGMACROS_INC

#include <fhglog/Logger.hpp>
#include <sstream>

namespace fhg { namespace log {

// TODO: perform some magic to discover whether we have __PRETTY_FUNCTION__ or not
// as it seems it's not macro
#define FHGLOG_FUNCTION __PRETTY_FUNCTION__

#define LOG_TRACE(logger, msg) do { using namespace fhg::log;\
                                    ::std::ostringstream osstr; osstr << msg; logger.log(LogEvent(LogLevel(LogLevel::TRACE), __FILE__, FHGLOG_FUNCTION, __LINE__, osstr.str())); } while(0)
#define LOG_DEBUG(logger, msg) do { using namespace fhg::log;\
                                    ::std::stringstream osstr; osstr << msg; logger.log(LogEvent(LogLevel(LogLevel::DEBUG), __FILE__, FHGLOG_FUNCTION, __LINE__, osstr.str())); } while(0)
#define LOG_INFO(logger, msg)  do { using namespace fhg::log;\
                                    ::std::stringstream osstr; osstr << msg; logger.log(LogEvent(LogLevel(LogLevel::INFO), __FILE__, FHGLOG_FUNCTION, __LINE__, osstr.str())); } while(0)
#define LOG_WARN(logger, msg)  do { using namespace fhg::log;\
                                    ::std::ostringstream osstr; osstr << msg; logger.log(LogEvent(LogLevel(LogLevel::WARN), __FILE__, FHGLOG_FUNCTION, __LINE__, osstr.str())); } while(0)
}}

#endif   /* ----- #ifndef FHG_LOG_LOGMACROS_INC  ----- */
