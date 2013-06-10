#include "DefaultConfiguration.hpp"

#include <fhglog/fhglog.hpp>
#include <fhglog/Appender.hpp>
#include <fhglog/Filter.hpp>
#include <fhglog/LogEvent.hpp>
#include <fhglog/CompoundAppender.hpp>
#include <fhglog/ThreadedAppender.hpp>
#include <fhglog/SynchronizedAppender.hpp>
#include <fhglog/StreamAppender.hpp>
#include <fhglog/SynchronizedAppender.hpp>
#include <fhglog/FileAppender.hpp>
#include <fhglog/CompoundAppender.hpp>
#include <fhglog/MemoryAppender.hpp>

#if defined(FHGLOG_WITH_REMOTE_LOGGING)
#   include <fhglog/remote/RemoteAppender.hpp>
#endif


#include <algorithm> // std::transform
#include <cctype>    // std::tolower

namespace fhg
{
  namespace log
  {
    DefaultConfiguration::DefaultConfiguration()
      : level_(LogLevel::DEF_LEVEL)
      , to_console_("")
      , to_file_("")
      , to_server_("")
      , fmt_string_("")
        // FIXME: broken if set to true
      , threaded_(false)
      , color_("auto")
      , disabled_(false)
      , synchronize_(false)
    {}

    void DefaultConfiguration::default_configuration ()
    {
      try
      {
        getLogger().removeAllAppenders();

        parse_environment();
        if (! check_config())
        {
          fallback_configuration();
        }

        configure();
      }
      catch (const std::exception& ex)
      {
        std::clog << "E: Could not configure the logging environment: " << ex.what() << std::endl;
      }
      catch (...)
      {
        std::clog << "E: Could not configure the logging environment: " << "unknown error" << std::endl;
      }
      std::clog.flush();
    }

    void DefaultConfiguration::parse_environment()
    {
      environment_t env = get_environment_variables();

      for (environment_t::const_iterator entry(env.begin()); entry != env.end(); ++entry)
      {
        std::string key(entry->first);
        std::string val(entry->second);
        if (key.find ("FHGLOG_") != std::string::npos)
        {
          // strip key and make lowercase
          key = key.substr(7);
          std::transform( key.begin()
                        , key.end()
                        , key.begin()
                        , tolower
                        );
          parse_key_value(key, val);
        }
      }
    }

    bool DefaultConfiguration::check_config()
    {
      if (disabled_) return true;

      if (to_console_.empty()
         && to_server_.empty()
         && to_file_.empty()
         )
      {
        return false;
      }

      return true;
    }

    void DefaultConfiguration::configure()
    {
      if (disabled_)
        return;

      std::string fmt (default_format::SHORT());

      StreamAppender::ColorMode color_mode (StreamAppender::COLOR_OFF);
      if (color_ == "auto")
        color_mode = StreamAppender::COLOR_AUTO;
      if (color_ == "off")
        color_mode = StreamAppender::COLOR_OFF;
      if (color_ == "on")
        color_mode = StreamAppender::COLOR_ON;

      if (fmt_string_.size())
      {
#ifdef FHGLOG_DEBUG_CONFIG
        std::clog << "D: setting format to \"" << fmt_string_ << "\"" << std::endl;
#endif
        if      (fmt_string_ == "full")    fmt = default_format::LONG();
        else if (fmt_string_ == "short")   fmt = default_format::SHORT();
        else if (fmt_string_ == "default") fmt = default_format::SHORT();
        else                               fmt = fmt_string_;

        check_format (fmt);
      }

      CompoundAppender::ptr_t compound_appender(new CompoundAppender("auto-config-appender"));

      compound_appender->addAppender (global_memory_appender ("system"));

      if (STDERR() == to_console_)
      {
        compound_appender->addAppender
          (Appender::ptr_t(new StreamAppender( "console"
                                             , std::cerr
                                             , fmt
                                             , color_mode
                                             )
                          )
          );
#ifdef FHGLOG_DEBUG_CONFIG
        std::clog << "D: logging to console: " << to_console_ << std::endl;
#endif
      }
      else if (STDOUT() == to_console_)
      {
        compound_appender->addAppender
          (Appender::ptr_t(new StreamAppender( "console"
                                             , std::cout
                                             , fmt
                                             , color_mode
                                             )
                          )
          );
#ifdef FHGLOG_DEBUG_CONFIG
        std::clog << "D: logging to console: " << to_console_ << std::endl;
#endif
      }
      else if (STDLOG() == to_console_)
      {
        compound_appender->addAppender
          (Appender::ptr_t(new StreamAppender( "console"
                                             , std::clog
                                             , fmt
                                             , color_mode
                                             )
                          )
          );
#ifdef FHGLOG_DEBUG_CONFIG
        std::clog << "D: logging to console: " << to_console_ << std::endl;
#endif
      }
      else if (to_console_.size() > 0)
      {
        std::clog << "W: invalid value for configuration value to_console: " << to_console_ << " assuming stderr" << std::endl;
        compound_appender->addAppender
          (Appender::ptr_t(new StreamAppender( "console"
                                             , std::cerr
                                             , fmt
                                             , color_mode
                                             )
                          )
          );
      }

      if (to_file_.size())
      {
        try
        {
          compound_appender->addAppender
            (Appender::ptr_t(new FileAppender( "log-file"
                                             , to_file_
                                             , fmt
                                             )
                            )
            );
#ifdef FHGLOG_DEBUG_CONFIG
          std::clog << "D: logging to file: " << to_file_ << std::endl;
#endif
        }
        catch (const std::exception &ex)
        {
          std::clog << "E: could not open log-file " << to_file_ << ": " << ex.what() << std::endl;
        }
      }

#if defined(FHGLOG_WITH_REMOTE_LOGGING)
      if (to_server_.size())
      {
        try
        {
          // TODO: split to_remote_ into host and port
          compound_appender->addAppender(Appender::ptr_t(new remote::RemoteAppender("log-server", to_server_)));
#ifdef FHGLOG_DEBUG_CONFIG
          std::clog << "D: logging to server: " << to_server_ << std::endl;
#endif
        }
        catch (const std::exception &ex)
        {
          std::clog << "E: could not create remote logger to: " << to_server_ << ": " << ex.what() << std::endl;
        }
      }
#endif

#ifdef FHGLOG_DEBUG_CONFIG
      std::clog << "D: loglevel set to " << level_ << std::endl;
#endif
      getLogger().setLevel(level_);

      if (threaded_)
      {
        getLogger().addAppender(Appender::ptr_t(new ThreadedAppender(compound_appender)));
      }
      else
      {
        if (synchronize_)
        {
          getLogger().addAppender(Appender::ptr_t(new SynchronizedAppender (compound_appender)));
        }
        else
        {
          getLogger().addAppender(compound_appender);
        }
      }
    }

    void DefaultConfiguration::fallback_configuration()
    {
      to_console_ = "stderr";
      to_file_ = "";
      to_server_ = "";
    }

    void DefaultConfiguration::parse_key_value(const std::string &key, const std::string &val)
    {
      if (key == "level")
      {
        level_ = LogLevel(val);
      }
      else if (key == "format")
      {
        fmt_string_ = val;
      }
      else if (key == "to_console")
      {
        to_console_ = val;
      }
      else if (key == "to_file")
      {
        to_file_ = val;
      }
      else if (key == "to_server")
      {
        to_server_ = val;
      }
      else if (key == "threaded" && (val == "yes" || val == "true" || val == "1"))
      {
        threaded_ = true;
      }
      else if (key == "color")
      {
        color_ = val;
      }
      else if (key == "synch")
      {
        synchronize_ = true;
      }
      else if (key == "disabled")
      {
#ifdef FHGLOG_DEBUG_CONFIG
        std::clog << "D: logging disabled due to environment FHGLOG_disabled" << std::endl;
#endif
        disabled_ = true;
      }
      else
      {
#ifdef FHGLOG_DEBUG_CONFIG
        std::clog << "D: ignoring key: " << key << std::endl;
#endif
      }
    }
  }
}
