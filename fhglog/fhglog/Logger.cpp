#include    "Logger.hpp"
#include    <stdexcept>
#include    <iostream>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>

using namespace fhg::log;

namespace state
{
  struct state_t
  {
    typedef boost::unordered_map<std::string, Logger::ptr_t> logger_map_t;

    Logger::ptr_t getLogger ( const std::string& name
                            , const std::string& base
                            )
    {
      boost::unique_lock<boost::recursive_mutex> const _ (_mutex);

      logger_map_t::iterator logger (_logger.find (name));

      if (logger == _logger.end())
      {
        Logger::ptr_t newLogger;

        if (name != "default")
        {
          newLogger.reset (new Logger (name, *getLogger (base, "default")));
        }
        else
        {
          newLogger.reset (new Logger (name));
        }

        logger = _logger.insert (std::make_pair (name, newLogger)).first;
      }

      return logger->second;
    }

    void terminate()
    {
      BOOST_FOREACH ( Logger::ptr_t const& logger
                    , _logger | boost::adaptors::map_values
                    )
      {
        logger->flush();
      }
    }

    ~state_t ()
    {
      boost::unique_lock<boost::recursive_mutex> _ (_mutex);

      _logger.clear ();
    }

  private:
    boost::recursive_mutex _mutex;
    logger_map_t _logger;
  };

  static state_t* static_state;

  state_t* get()
  {
    if (! static_state)
    {
      static_state = new state_t;
    }

    return static_state;
  }

  void clear()
  {
    delete static_state;

    static_state = 0;
  }
}

namespace fhg
{
  namespace log
  {
    void terminate()
    {
      state::get()->terminate ();
      state::clear();
    }
  }
}

Logger::ptr_t Logger::get()
{
  return get ("default", "default");
}

Logger::ptr_t Logger::get ( const std::string& name, const std::string& base)
{
  return state::get()->getLogger (name, base);
}

Logger::Logger (const std::string& name)
  : name_ (name)
  , lvl_ (LogLevel (LogLevel::DEF_LEVEL))
  , filter_ (new NullFilter())
{}

Logger::Logger ( const std::string& name
               , const Logger& inherit_from
               )
  : name_ (name)
  , lvl_ (inherit_from.lvl_)
  , filter_ (inherit_from.filter_)
{
  BOOST_FOREACH (Appender::ptr_t const& appender, inherit_from.appenders_)
  {
    addAppender (appender);
  }
}

void Logger::setLevel (const LogLevel& level)
{
  lvl_ = level;
}

void Logger::log (const LogEvent& event)
{
  if (isLevelEnabled (event.severity()))
  {
    event.trace (name_);

    BOOST_FOREACH (Appender::ptr_t const& appender, appenders_)
    {
      appender->append(event);
    }
  }

  if (event.severity() == LogLevel::FATAL)
  {
    throw std::runtime_error (event.message());
  }
}

void Logger::flush()
{
  BOOST_FOREACH (Appender::ptr_t const& appender, appenders_)
  {
    appender->flush();
  }
}

void Logger::addAppender (Appender::ptr_t appender)
{
  appenders_.push_back(appender);
}
