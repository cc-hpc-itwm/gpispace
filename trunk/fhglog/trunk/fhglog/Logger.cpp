#include    "Logger.hpp"
#include    <stdexcept>
#include    <iostream>

using namespace fhg::log;

/*
 * Logger implementation
 *
 */
const Logger::ptr_t &Logger::get()
{
  static Logger::ptr_t logger_(new Logger(""));
  return logger_;
}

const Logger::ptr_t &Logger::get(const std::string &name)
{
  return get()->get_logger(name);
}

Logger::Logger(const std::string &name)
  : name_(name), lvl_(LogLevel(LogLevel::UNSET))
{
}

Logger::Logger(const std::string &name, const Logger &parent)
  : name_(name), lvl_(parent.getLevel())
{
  for (appender_list_t::const_iterator appender(parent.appenders_.begin()); appender != parent.appenders_.end(); ++appender)
  {
    addAppender(*appender);
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

const Logger::ptr_t &Logger::get_logger(const std::string &name)
{
  std::string::size_type spos(0);
  std::string::size_type epos(name.find_first_of('.'));

  if (std::string::npos == epos)
  {
    return get_add_logger(name);
  }
  else
  {
    return get_add_logger(name.substr(spos, epos), name.substr(epos+1, name.size()));
  }
}

const Logger::ptr_t &Logger::get_add_logger(const std::string &name, const std::string &rest)
{
  logger_map_t::iterator logger(loggers_.find(name));
  if (logger == loggers_.end())
  {
    Logger::ptr_t newLogger(new Logger(name, *this)); // inherit config from this logger
    logger = loggers_.insert(std::make_pair(name, newLogger)).first;
  }
  if (rest == "") return logger->second;
  else return logger->second->get_logger(rest);
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

void Logger::log(const LogEvent &event) const
{
  if (! isLevelEnabled(event.severity()))
    return;

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

