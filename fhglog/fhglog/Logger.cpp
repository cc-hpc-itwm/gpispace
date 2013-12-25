#include    "Logger.hpp"
#include    <stdexcept>
#include    <iostream>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

#include <boost/foreach.hpp>

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
  : name_ (a_name)
  , lvl_ (inherit_from.lvl_)
  , filter_ (inherit_from.filter_)
{
  BOOST_FOREACH (Appender::ptr_t const& appender, inherit_from.appenders_)
  {
    addAppender (appender);
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

void Logger::log(const LogEvent &event)
{
  if (! isLevelEnabled(event.severity()))
    return;
  event.trace(name());

  BOOST_FOREACH (Appender::ptr_t const& appender, appenders_)
  {
    appender->append(event);
  }

  if (event.severity() == LogLevel::FATAL)
  {
    throw std::runtime_error (event.message());
  }
}

void Logger::flush (void)
{
  BOOST_FOREACH (Appender::ptr_t const& appender, appenders_)
  {
    appender->flush();
  }
}

const Appender::ptr_t &Logger::addAppender(const Appender::ptr_t &appender)
{
  appenders_.push_back(appender);
  return appender;
}
