#include    "Logger.hpp"
#include    <stdexcept>
#include    <iostream>

using namespace fhg::log;

/*
 * Logger implementation
 *
 */
Logger &Logger::getRootLogger()
{
  static Logger logger_("");
  return logger_;
}

LoggerApi Logger::get(const std::string &name)
{
  // TODO: do some tree computation here and return a meaningfull logger
  return LoggerApi(&Logger::getRootLogger());
}

Logger::Logger(const std::string &name)
  : name_(name), lvl_(LogLevel(LogLevel::UNSET))
{
}

const std::string &Logger::name() const
{
  return name_;
}

void Logger::setLevel(const LogLevel &level)
{
  lvl_ = level;
}

bool Logger::isLevelEnabled(const LogLevel &level)
{
  // TODO: inherit the level from the parent logger if the level was not set
  return (lvl_ != LogLevel::UNSET) ? lvl_ <= level : true;
}

void Logger::log(const LogEvent &event)
{
  if (! isLevelEnabled(event.severity()))
    return;

  for (appender_list_t::iterator it(appenders_.begin());
       it != appenders_.end();
       ++it)
  {
    try
    {
      (*it)->append(event);
    } catch (...)
    {
      std::clog << "could not append log event to appender: " << (*it)->name() << std::endl;
    }
  }
}

void Logger::addAppender(Appender::ptr_t appender)
{
  appenders_.push_back(appender);
}

Appender::ptr_t Logger::getAppender(const std::string &appender_name)
{
  for (appender_list_t::iterator it(appenders_.begin());
       it != appenders_.end();
       ++it)
  {
    if (appender_name == (*it)->name())
    {
      return *it;
    }
  }
  throw std::runtime_error("no matching appender could be found!");
}

void Logger::removeAppender(const std::string &appender_name)
{
  for (appender_list_t::iterator it(appenders_.begin());
       it != appenders_.end();
       ++it)
  {
    if (appender_name == (*it)->name())
    {
      appenders_.erase(it);
      break;
    }
  }
}

