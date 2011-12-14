#include    "Logger.hpp"
#include    <stdexcept>
#include    <iostream>
#include    <pthread.h>

using namespace fhg::log;

/*
 * Logger implementation
 *
 */
const Logger::ptr_t &Logger::get()
{
  return get("default");
}

const Logger::ptr_t &Logger::get(const std::string &a_name)
{
  typedef std::map<std::string, Logger::ptr_t> logger_map_t;
  static logger_map_t loggers_;
  static pthread_mutex_t lock_ = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

  pthread_mutex_lock(&lock_);
  logger_map_t::iterator logger(loggers_.find(a_name));
  if (logger == loggers_.end())
  {
    if (a_name != "default")
    {
      Logger::ptr_t newLogger(new Logger(a_name, *get("default")));
      logger = loggers_.insert(std::make_pair(a_name, newLogger)).first;
    }
    else
    {
      Logger::ptr_t newLogger(new Logger(a_name));
      logger = loggers_.insert(std::make_pair(a_name, newLogger)).first;
    }
  }

  pthread_mutex_unlock(&lock_);
  return logger->second;
}

Logger::Logger(const std::string &a_name)
  : name_(a_name), lvl_(LogLevel(LogLevel::UNSET)), filter_(new NullFilter())
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

bool Logger::isLevelEnabled(const LogLevel &level) const
{
  // TODO: inherit the level from the parent logger if the level was not set
  return (lvl_ != LogLevel::UNSET) ? lvl_ <= level : true;
}

bool Logger::isFiltered(const LogEvent &evt) const
{
  return (*filter_)(evt);
}

void Logger::log(const LogEvent &event) const
{
  if (! isLevelEnabled(event.severity()))
    return;
  event.finish();
  event.logged_via(name());

  for (appender_list_t::const_iterator it(appenders_.begin());
       it != appenders_.end();
       ++it)
  {
    try
    {
      (*it)->append(event);
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

