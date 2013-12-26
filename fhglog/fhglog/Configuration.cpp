// bernd.loerwald@itwm.fraunhofer.de

#include <fhglog/Configuration.hpp>

#include <fhglog/Appender.hpp>
#include <fhglog/appender/compound.hpp>
#include <fhglog/appender/file.hpp>
#include <fhglog/Filter.hpp>
#include <fhglog/event.hpp>
#include <fhglog/level.hpp>
#include <fhglog/appender/memory.hpp>
#include <fhglog/appender/stream.hpp>
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
          : level_ (INFO)
          , to_console_()
          , to_file_()
          , to_server_()
          , fmt_string_ (default_format::SHORT())
          , color_ (StreamAppender::COLOR_OFF)
          , disabled_ (false)
        {}

        void parse_environment();
        void parse_key_value (const std::string& key, const std::string& val);

        void configure() const;

        fhg::log::Level level_;
        std::string to_console_;
        std::string to_file_;
        std::string to_server_;
        std::string fmt_string_;
        StreamAppender::ColorMode color_;
        bool disabled_;
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
      {
        if (key == "level")
        {
          level_ = from_string (val);
        }
        else if (key == "format")
        {
          fmt_string_ = val == "full" ? default_format::LONG()
                      : val == "short" ? default_format::SHORT()
                      : val == "default" ? default_format::SHORT()
                      : val;

          check_format (fmt_string_);
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
        else if (key == "color")
        {
          color_ = val == "off" ? StreamAppender::COLOR_OFF
                 : val == "on" ? StreamAppender::COLOR_ON
                 : throw std::runtime_error
                     ("expected: 'on' or 'off' for key 'color'");
        }
        else if (key == "disabled")
        {
          disabled_ = true;
        }
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
        }

        if (to_file_.size())
        {
          compound_appender->addAppender
            (Appender::ptr_t (new FileAppender (to_file_, fmt_string_)));
        }

        if (to_server_.size())
        {
          // TODO: split to_remote_ into host and port
          compound_appender->addAppender
            (Appender::ptr_t (new remote::RemoteAppender (to_server_)));
        }

        getLogger().setLevel (level_);

        getLogger().addAppender
          ( compound_appender
          );
      }
    }

    void configure()
    {
      DefaultConfiguration conf;

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
    }
  }
}
