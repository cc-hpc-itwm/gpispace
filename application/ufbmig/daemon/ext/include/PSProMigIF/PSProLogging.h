#ifndef PSPROLOGGING_H
#define PSPROLOGGING_H

#ifdef HAVE_LOG4CPLUS

#include <log4cplus/logger.h>

/// Get the default root logger. Use this if you don't have your own logger (created using DECLARE_LOGGER() ):
#define PSPRO_LOGGER (log4cplus::Logger::getRoot())


/// Create your own logger as a file-global variable:
#define DECLARE_LOGGER(_logger)     static log4cplus::Logger _logger __attribute__((unused)) = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT(#_logger))

/// Set the current log level of a logger, messages below the current level will not be logged
#define SET_LOG_LEVEL(_logger, _level)  _logger.setLogLevel(log4cplus::_level##_LOG_LEVEL)


/** Log with arbitrary log messages at the different log levels:
 * TRACE messages are for developers
 * INFO, ERROR and FATAL might stay enabled also in the release version, so users could send us the log file
 * with these messages if something goes wrong.
 */
#define TRACE_PRINTF(_logger, ...)  LOG4CPLUS_TRACE_PRINTF(_logger, __VA_ARGS__)
#define INFO_PRINTF(_logger, ...)   LOG4CPLUS_INFO_PRINTF(_logger, __VA_ARGS__)
#define ERROR_PRINTF(_logger, ...)  LOG4CPLUS_ERROR_PRINTF(_logger, __VA_ARGS__)
#define FATAL_PRINTF(_logger, ...)  LOG4CPLUS_FATAL_PRINTF(_logger, __VA_ARGS__)


/// Print an ENTER and EXIT function log, only for loglevel TRACE:
#define TRACE_METHOD(_logger)       LOG4CPLUS_TRACE_METHOD(_logger, __PRETTY_FUNCTION__)

/// Print an ENTER function log, only for loglevel INFO:
#define INFO_METHOD()               LOG4CPLUS_INFO_STR(PSPRO_LOGGER, __PRETTY_FUNCTION__)


/// Print the current function name and line number:
#define TRACE_LINE(_logger)         TRACE_PRINTF(_logger, "%s %d", __PRETTY_FUNCTION__, __LINE__)


/// Print the value of a variable at TRACE loglevel:
#define TRACE_VAR(_logger, _var)                         LOG4CPLUS_TRACE(_logger, __PRETTY_FUNCTION__ << " " <<__LINE__\
                         << ": " << #_var << "=" << _var )

#define TRACE_VAR2(_logger, _var1, _var2)                LOG4CPLUS_TRACE(_logger, __PRETTY_FUNCTION__ << " " <<__LINE__ \
                         << ": " << #_var1 << "=" << _var1 << ", " << #_var2 << "=" << _var2 )

#define TRACE_VAR3(_logger, _var1, _var2, _var3)         LOG4CPLUS_TRACE(_logger, __PRETTY_FUNCTION__ << " " <<__LINE__\
                         << ": " << #_var1 << "=" << _var1 << ", " << #_var2 << "=" << _var2 \
                         << ", " << #_var3 << "=" << _var3 )

#define TRACE_VAR4(_logger, _var1, _var2, _var3, _var4)  LOG4CPLUS_TRACE(_logger, __PRETTY_FUNCTION__ << " " <<__LINE__\
                         << ": " << #_var1 << "=" << _var1 << ", " << #_var2 << "=" << _var2 \
                         << ", " << #_var3 << "=" << _var3 << ", " << #_var4 << "=" << _var4)


// see http://www.decompile.com/cpp/faq/file_and_line_error_string.htm for an explanation of the following two lines:
#define __TO_STRING2(_x) #_x
#define __TO_STRING(_x) __TO_STRING2(_x)

/// Create a log message (FATAL level) if the expression _exp is no true:
#define LOG_ASSERT(_logger, _exp) _logger.assertion( _exp , "ASSERTION  \'" #_exp "\'  FAILED ! " \
                                                            "(in " __FILE__ ":" __TO_STRING(__LINE__) ")" )

/* currently the logging cannot be used in tests, maybe we need something like the following:
#include <log4cplus/consoleappender.h>
#include <log4cplus/loglevel.h>
#include <log4cplus/tstring.h>
#include <log4cplus/helpers/threads.h>

#define SETUP_TEST_STDERR_LOGGING                                   \
{                                                                   \
  log4cplus::Logger::getRoot().setLogLevel(log4cplus::TRACE_LOG_LEVEL);                   \
  log4cplus::SharedAppenderPtr append_2(new log4cplus::ConsoleAppender(true, true));      \
  append_2->setName(LOG4CPLUS_TEXT("Stderr"));                      \
  log4cplus::Logger::getRoot().addAppender(append_2);                          \
}
*/

#else

#include <stdio.h>
#include <iostream>

#define PSPRO_LOGGER


/// Create your own logger as a file-global variable:
#define DECLARE_LOGGER(_logger)     static int _logger __attribute__((unused)) = 0

/// Set the current log level of a logger, messages below the current level will not be logged
#define SET_LOG_LEVEL(_logger, _level)  {}


/** Log with arbitrary log messages at the different log levels:
 * TRACE messages are for developers
 * INFO, ERROR and FATAL might stay enabled also in the release version, so users could send us the log file
 * with these messages if something goes wrong.
 */
#define TRACE_PRINTF(_logger, ...)  {fprintf(stderr, "TRACE: "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n");}
#define INFO_PRINTF(_logger, ...)   {fprintf(stderr, "INFO: "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n");}
#define ERROR_PRINTF(_logger, ...)  {fprintf(stderr, "ERROR: "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n");}
#define FATAL_PRINTF(_logger, ...)  {fprintf(stderr, "FATAL: "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n");}


/// Print an ENTER and EXIT function log, only for loglevel TRACE:
#define TRACE_METHOD(_logger)       fprintf(stderr, "TRACE: %s\n", __PRETTY_FUNCTION__)

/// Print an ENTER function log, only for loglevel INFO:
#define INFO_METHOD()               fprintf(stderr, "INFO: %s\n", __PRETTY_FUNCTION__)


/// Print the current function name and line number:
#define TRACE_LINE(_logger)     {fprintf(stderr, "TRACE: "); fprintf(stderr, "%s %d\n", __PRETTY_FUNCTION__, __LINE__);}


/// Print the value of a variable at TRACE loglevel:
#define TRACE_VAR(_logger, _var)                       std::cerr << "TRACE: " << __PRETTY_FUNCTION__ << " " <<__LINE__\
                         << ": " << #_var << "=" << _var << std::endl;

#define TRACE_VAR2(_logger, _var1, _var2)             std::cerr << "TRACE: " << __PRETTY_FUNCTION__ << " " <<__LINE__ \
                         << ": " << #_var1 << "=" << _var1 << ", " << #_var2 << "=" << _var2 << std::endl;

#define TRACE_VAR3(_logger, _var1, _var2, _var3)        std::cerr << "TRACE: " << __PRETTY_FUNCTION__ << " " <<__LINE__\
                         << ": " << #_var1 << "=" << _var1 << ", " << #_var2 << "=" << _var2 \
                         << ", " << #_var3 << "=" << _var3 << std::endl;

#define TRACE_VAR4(_logger, _var1, _var2, _var3, _var4) std::cerr << "TRACE: " << __PRETTY_FUNCTION__ << " " <<__LINE__\
                         << ": " << #_var1 << "=" << _var1 << ", " << #_var2 << "=" << _var2 \
                         << ", " << #_var3 << "=" << _var3 << ", " << #_var4 << "=" << _var4 << std::endl;


#endif


#endif
