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
  static Logger::ptr_t logger_(new Logger("", ""));
  return logger_;
}

const Logger::ptr_t &Logger::get(const std::string &name)
{
  return get()->get_logger(name);
}

Logger::Logger(const std::string &a_name, const std::string &a_parent)
  : name_(a_parent + "." + a_name), parent_(a_parent), lvl_(LogLevel(LogLevel::UNSET)), filter_(new NullFilter())
{
}

Logger::Logger(const std::string &a_name, const Logger &a_parent)
  : name_(a_parent.name() + "." + a_name), parent_(a_parent.name()), lvl_(a_parent.getLevel()), filter_(a_parent.getFilter())
{
  for (appender_list_t::const_iterator appender(a_parent.appenders_.begin()); appender != a_parent.appenders_.end(); ++appender)
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

const Logger::ptr_t &Logger::get_logger(const std::string &a_name)
{
  std::string::size_type spos(0);
  std::string::size_type epos(a_name.find_first_of('.'));

  if (std::string::npos == epos)
  {
    return get_add_logger(a_name);
  }
  else
  {
    return get_add_logger(a_name.substr(spos, epos), a_name.substr(epos+1, a_name.size()));
  }
}

const Logger::ptr_t &Logger::get_add_logger(const std::string &a_name, const std::string &rest)
{
  logger_map_t::iterator logger(loggers_.find(a_name));
  if (logger == loggers_.end())
  {
    Logger::ptr_t newLogger(new Logger(a_name, *this)); // inherit config from this logger
    logger = loggers_.insert(std::make_pair(a_name, newLogger)).first;
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

