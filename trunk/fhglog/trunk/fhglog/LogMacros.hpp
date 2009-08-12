#ifndef  FHG_LOG_LOGMACROS_INC
#define  FHG_LOG_LOGMACROS_INC

#include <fhglog/Logger.hpp>
#include <sstream>

namespace fhg { namespace log {

#define LOG_TRACE(logger, msg) do { using namespace fhg::log;\
                                    ::std::stringstream sstr; sstr << msg; logger.log(LogEvent(LogLevel(LogLevel::TRACE), __FILE__, __FUNCTION__, __LINE__, sstr.str())); } while(0)
#define LOG_DEBUG(logger, msg) do { using namespace fhg::log;\
                                    ::std::stringstream sstr; sstr << msg; logger.log(LogEvent(LogLevel(LogLevel::DEBUG), __FILE__, __FUNCTION__, __LINE__, sstr.str())); } while(0)
#define LOG_INFO(logger, msg) do  { using namespace fhg::log;\
                                    ::std::stringstream sstr; sstr << msg; logger.log(LogEvent(LogLevel(LogLevel::INFO), __FILE__, __FUNCTION__, __LINE__, sstr.str())); } while(0)
#define LOG_WARN(logger, msg) do  { using namespace fhg::log;\
                                    ::std::stringstream sstr; sstr << msg; logger.log(LogEvent(LogLevel(LogLevel::WARN), __FILE__, __FUNCTION__, __LINE__, sstr.str())); } while(0)
}}

#endif   /* ----- #ifndef FHG_LOG_LOGMACROS_INC  ----- */
