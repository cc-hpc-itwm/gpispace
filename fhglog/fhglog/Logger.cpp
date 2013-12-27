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

    Logger::ptr_t getLogger (const std::string& name)
    {
      boost::unique_lock<boost::recursive_mutex> const _ (_mutex);

      logger_map_t::const_iterator const logger (_logger.find (name));

      return (logger != _logger.end()) ? logger->second
        : _logger.insert
          ( std::make_pair
            ( name
            , Logger::ptr_t ( name != "default"
                            ? new Logger (name, *getLogger ("default"))
                            : new Logger (name)
                            )
            )
          ).first->second;
    }

  private:
    boost::recursive_mutex _mutex;
    logger_map_t _logger;
  };

  state_t& get()
  {
    static state_t s;

    return s;
  }
}

Logger::ptr_t Logger::get()
{
  return get ("default");
}

Logger::ptr_t Logger::get (const std::string& name)
{
  return state::get().getLogger (name);
}

Logger::Logger (const std::string& name)
  : name_ (name)
  , lvl_ (INFO)
{}

Logger::Logger ( const std::string& name
               , const Logger& inherit_from
               )
  : name_ (name)
  , lvl_ (inherit_from.lvl_)
  , appenders_ (inherit_from.appenders_)
{}

void Logger::setLevel (const Level& level)
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

  if (event.severity() == FATAL)
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
