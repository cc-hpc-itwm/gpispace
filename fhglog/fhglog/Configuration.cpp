// bernd.loerwald@itwm.fraunhofer.de

#include <fhglog/Configuration.hpp>

#include <fhglog/fhglog-config.hpp>
#include <fhglog/Appender.hpp>
#include <fhglog/CompoundAppender.hpp>
#include <fhglog/FileAppender.hpp>
#include <fhglog/Filter.hpp>
#include <fhglog/LogEvent.hpp>
#include <fhglog/LogLevel.hpp>
#include <fhglog/MemoryAppender.hpp>
#include <fhglog/StreamAppender.hpp>
#include <fhglog/SynchronizedAppender.hpp>
#include <fhglog/ThreadedAppender.hpp>
#include <fhglog/fhglog.hpp>
#include <fhglog/remote/RemoteAppender.hpp>

#include <fhg/util/read_bool.hpp>
#include <fhg/util/split.hpp>

#include <algorithm> // std::transform
#include <cctype>    // std::tolower
#include <iostream>

#ifdef __APPLE__
#include <crt_externs.h> // _NSGetEnviron
#else
#include <unistd.h> // char **environ
#endif

namespace fhg
{
  namespace log
  {
    namespace
    {
      class DefaultConfiguration
      {
      public:
        DefaultConfiguration()
          : level_ (LogLevel::DEF_LEVEL)
          , to_console_()
          , to_file_()
          , to_server_()
          , fmt_string_ (default_format::SHORT())
            // FIXME: broken if set to true
          , threaded_ (false)
          , color_ (StreamAppender::COLOR_AUTO)
          , disabled_ (false)
          , synchronize_ (false)
        {}

        void parse_environment();
        void parse_key_value (const std::string& key, const std::string& val);

        void configure() const;

        fhg::log::LogLevel level_;
        std::string to_console_;
        std::string to_file_;
        std::string to_server_;
        std::string fmt_string_;
        bool threaded_;
        StreamAppender::ColorMode color_;
        bool disabled_;
        bool synchronize_;
      };

      void DefaultConfiguration::parse_environment()
      {
        for ( char** env_p
#ifdef __APPLE__
              (*_NSGetEnviron())
#else
              (environ)
#endif
            ; env_p != NULL && (*env_p != NULL)
            ; ++env_p
            )
        {
          const std::pair<std::string, std::string> key_value
            (fhg::util::split_string (*env_p, "="));

          if (key_value.first.find ("FHGLOG_") != std::string::npos)
          {
            std::string key (key_value.first.substr (7));
            std::transform (key.begin(), key.end(), key.begin(), tolower);
            parse_key_value(key, key_value.second);
          }
        }
      }

      void DefaultConfiguration::parse_key_value
        (const std::string& key, const std::string& val)
      try
      {
        if (key == "level")
        {
          level_ = LogLevel (val);
        }
        else if (key == "format")
        {
          fmt_string_ = val == "full" ? default_format::LONG()
                      : val == "short" ? default_format::SHORT()
                      : val == "default" ? default_format::SHORT()
                      : val;

          check_format (fmt_string_);

#ifdef FHGLOG_DEBUG_CONFIG
          std::clog << "D: setting format to \"" << val << "\"" << std::endl;
#endif
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
        else if (key == "threaded")
        {
          threaded_ = fhg::util::read_bool (val);
        }
        else if (key == "color")
        {
          color_ = val == "auto" ? StreamAppender::COLOR_AUTO
                 : val == "off" ? StreamAppender::COLOR_OFF
                 : val == "on" ? StreamAppender::COLOR_ON
                 : throw std::runtime_error ("expected: 'auto', 'on' or 'off'");
        }
        else if (key == "synch")
        {
          synchronize_ = true;
        }
        else if (key == "disabled")
        {
#ifdef FHGLOG_DEBUG_CONFIG
          std::clog << "D: logging disabled due to environment FHGLOG_disabled"
                    << std::endl;
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
      catch (const std::exception& ex)
      {
        std::clog << "E: invalid value '" << val << "' for key '" << key
                  << "':" << ex.what() << std::endl;
      }


      void DefaultConfiguration::configure() const
      {
        CompoundAppender::ptr_t compound_appender (new CompoundAppender);

        compound_appender->addAppender (global_memory_appender());

        if (to_console_.size())
        {
          compound_appender->addAppender
            ( Appender::ptr_t ( new StreamAppender
                                ( "stdout" == to_console_ ? std::cout
                                : "stdlog" == to_console_ ? std::clog
                                : std::cerr
                                , fmt_string_
                                , color_
                                )
                            )
            );

          if (  "stderr" != to_console_
             && "stdout" != to_console_
             && "stdlog" != to_console_
             )
          {
            std::clog << "W: invalid value for configuration value to_console: "
                      << to_console_ << " assuming stderr" << std::endl;
          }
#ifdef FHGLOG_DEBUG_CONFIG
          else
          {
            std::clog << "D: logging to console: " << to_console_ << std::endl;
          }
#endif
        }

        if (to_file_.size())
        {
          try
          {
            compound_appender->addAppender
              (Appender::ptr_t (new FileAppender (to_file_, fmt_string_)));
#ifdef FHGLOG_DEBUG_CONFIG
            std::clog << "D: logging to file: " << to_file_ << std::endl;
#endif
          }
          catch (const std::exception& ex)
          {
            std::clog << "E: could not open log-file " << to_file_ << ": "
                      << ex.what() << std::endl;
          }
        }

        if (to_server_.size())
        {
          try
          {
            // TODO: split to_remote_ into host and port
            compound_appender->addAppender
              (Appender::ptr_t (new remote::RemoteAppender (to_server_)));
#ifdef FHGLOG_DEBUG_CONFIG
            std::clog << "D: logging to server: " << to_server_ << std::endl;
#endif
          }
          catch (const std::exception &ex)
          {
            std::clog << "E: could not create remote logger to: "
                      << to_server_ << ": " << ex.what() << std::endl;
          }
        }

#ifdef FHGLOG_DEBUG_CONFIG
        std::clog << "D: loglevel set to " << level_ << std::endl;
#endif
        getLogger().setLevel (level_);

        getLogger().addAppender
          ( threaded_ ? Appender::ptr_t (new ThreadedAppender (compound_appender))
          : synchronize_ ? Appender::ptr_t (new SynchronizedAppender (compound_appender))
          : compound_appender
          );
      }
    }

    void configure()
    {
#if FHGLOG_DISABLE_LOGGING != 1

      DefaultConfiguration conf;

#ifdef FHGLOG_DEBUG_CONFIG
      std::clog << "I: performing default logging configuration" << std::endl;
#endif

      getLogger().removeAllAppenders();

      conf.parse_environment();

      if (conf.disabled_)
      {
        return;
      }

      if (conf.to_console_.empty() && conf.to_server_.empty() && conf.to_file_.empty())
      {
        conf.to_console_ = "stderr";
      }

      conf.configure();

#endif
    }

    void configure (int ac, char *av[])
    {
      // parameters currently ignored
      configure();
    }
  }
}