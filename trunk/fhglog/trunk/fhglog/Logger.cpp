#include    "Logger.hpp"
#include    <stdexcept>
#include    <iostream>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include "error_handler.hpp"

using namespace fhg::log;

/*
 * Logger implementation
 *
 */
Logger::ptr_t Logger::get()
{
  return get("default");
}

Logger::ptr_t Logger::get(const std::string &a_name, const std::string & base)
{
  typedef boost::unordered_map<std::string, Logger::ptr_t> logger_map_t;
  typedef boost::recursive_mutex mutex_type;
  typedef boost::lock_guard<mutex_type> lock_type;

  static logger_map_t loggers_;
  static mutex_type mtx_;

  lock_type lock(mtx_);
  logger_map_t::iterator logger(loggers_.find(a_name));
  if (logger == loggers_.end())
  {
    if (a_name != "default")
    {
      Logger::ptr_t newLogger(new Logger(a_name, *get(base)));
      logger = loggers_.insert(std::make_pair(a_name, newLogger)).first;
    }
    else
    {
      Logger::ptr_t newLogger(new Logger(a_name));
      logger = loggers_.insert(std::make_pair(a_name, newLogger)).first;
    }
  }

  return logger->second;
}

Logger::Logger(const std::string &a_name)
  : name_(a_name), lvl_(LogLevel(LogLevel::DEF_LEVEL)), filter_(new NullFilter())
{
}

Logger::Logger(const std::string &a_name, const Logger &inherit_from)
  : name_(a_name), lvl_(inherit_from.getLevel()), filter_(inherit_from.getFilter())
{
  for (appender_list_t::const_iterator it(inherit_from.appenders_.begin()); it != inherit_from.appenders_.end(); ++it)
  {
    addAppender(*it);
  }
}


const std::string &Logger::name() const
{
  return name_;
}

void Logger::setLevel(const LogLevel &level)
{
  lvl_ = level;
}

const LogLevel &Logger::getLevel() const
{
  return lvl_;
}

void Logger::log(const LogEvent &event)
{
  if (! isLevelEnabled(event.severity()))
    return;
  event.finish();
  event.logged_via(name());

  bool logged (false);
  for (appender_list_t::const_iterator it(appenders_.begin());
       it != appenders_.end();
       ++it)
  {
    try
    {
      (*it)->append(event);
      logged = true;
    }
    catch (const std::exception &ex)
    {
      std::clog << "could not append log event to appender " << (*it)->name() << ": " << ex.what() << std::endl;
    }
    catch (...)
    {
      std::clog << "could not append log event to appender " << (*it)->name() << ": unknown errror" << std::endl;
    }
  }

  if (event.severity() == LogLevel::FATAL)
  {
    if (! logged)
    {
      std::cerr << "logger " << name() << " got unhandled FATAL message: "
                << event.file() << ":" << event.line() << " - " << event.message()
                << std::endl;
    }
    fhg::log::error_handler();
  }
}

void Logger::flush (void)
{
  for (appender_list_t::const_iterator it(appenders_.begin());
       it != appenders_.end();
       ++it)
  {
    try
    {
      (*it)->flush ();
    }
    catch (std::exception const & ex)
    {
      std::clog << "could not flush " << (*it)->name() << ": " << ex.what() << std::endl;
    }
  }
}

bool Logger::hasAppender (void) const
{
  return ! appenders_.empty();
}
const Appender::ptr_t &Logger::addAppender(const Appender::ptr_t &appender)
{
  appenders_.push_back(appender);
  return appender;
}

const Filter::ptr_t &Logger::getFilter() const
{
  return filter_;
}

const Appender::ptr_t &Logger::getAppender(const std::string &appender_name) const
{
  for (appender_list_t::const_iterator it(appenders_.begin());
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

void Logger::removeAllAppenders()
{
  appenders_.clear();
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

