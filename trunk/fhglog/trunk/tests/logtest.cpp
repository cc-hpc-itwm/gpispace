/*
 * =====================================================================================
 *
 *       Filename:  logtest.cpp
 *
 *    Description:  testing tool for the logging framework
 *
 *        Version:  1.0
 *        Created:  08/11/2009 11:26:47 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include    <fhglog/fhglog.hpp>

int main(int argc, char **argv)
{
  using namespace fhg::log;
  LoggerApi logger(Logger::get("test"));
  logger.setLevel(LogLevel::WARN);
  logger.addAppender(Appender::ptr_t(new StreamAppender("console-long")))->setFormat(Formatter::DefaultFormatter());
  logger.addAppender(Appender::ptr_t(new StreamAppender("console-short")))->setFormat(Formatter::ShortFormatter());

  logger.log(LogEvent(LogLevel::TRACE
                            , __FILE__
                            , FHGLOG_FUNCTION
                            , __LINE__
                            , "trace message"));

  logger.log(LogEvent(LogLevel::DEBUG
                            , __FILE__
                            , FHGLOG_FUNCTION
                            , __LINE__
                            , "debug message"));

  logger.log(LogEvent(LogLevel::WARN
                            , __FILE__
                            , FHGLOG_FUNCTION
                            , __LINE__
                            , "warn message"));

  logger.log(LogEvent(LogLevel::ERROR
                            , __FILE__
                            , FHGLOG_FUNCTION
                            , __LINE__
                            , "error message"));

  LOG_DEBUG(logger, "this is an error:" << 42);
  LOG_WARN(logger, "this is a warning:" << 42);
}
