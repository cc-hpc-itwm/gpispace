#include    "Logger.hpp"
#include    <stdexcept>
#include    <iostream>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include "error_handler.hpp"

using namespace fhg::log;

namespace state
{
  struct state_t
  {
    typedef boost::unordered_map<std::string, Logger::ptr_t> logger_map_t;
    typedef boost::recursive_mutex mutex_type;
    typedef boost::unique_lock<mutex_type> lock_type;

    Logger::ptr_t getLogger(const std::string &a_name, const std::string & base)
    {
      lock_type lock(m_mutex);
      logger_map_t::iterator logger(m_loggers.find(a_name));
      if (logger == m_loggers.end())
      {
        Logger::ptr_t newLogger;
        if (a_name != "default")
        {
          newLogger.reset (new Logger(a_name, *getLogger(base, "default")));
        }
        else
        {
          newLogger.reset (new Logger(a_name));
        }
        assert (newLogger);

        logger = m_loggers.insert(std::make_pair(a_name, newLogger)).first;
      }
      return logger->second;
    }

    void terminate ()
    {
      logger_map_t::iterator logger (m_loggers.begin());
      logger_map_t::iterator end (m_loggers.end());

      while (logger != end)
      {
        logger->second->flush();
        ++logger;
      }
    }

    ~state_t ()
    {
      lock_type lock(m_mutex);
      m_loggers.clear ();
    }

  private:
    mutex_type   m_mutex;
    logger_map_t m_loggers;
  };

  //  typedef fhg::log::shared_ptr<state_t> state_ptr;
  typedef state_t* state_ptr;
  static state_ptr static_state;

  state_ptr get ()
  {
    if (! static_state)
      static_state = state_ptr (new state_t);
    return static_state;
  }

  void clear ()
  {
    delete static_state;
    static_state = 0;
  }
}

namespace fhg
{
  namespace log
  {
    void terminate ()
    {
      state::get ()->terminate ();
      state::clear ();
    }
  }
}

/*
 * Logger implementation
 *
 */
Logger::ptr_t Logger::get()
{
  return get("default", "default");
}

Logger::ptr_t Logger::get(const std::string &a_name, const std::string & base)
{
  return state::get ()->getLogger (a_name, base);
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

Logger::~Logger ()
{
  try
  {
    flush ();
  }
  catch (...)
  {
    // nothing can be done
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
  event.trace(name());

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
                << event.path() << ":" << event.line() << " - " << event.message()
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
