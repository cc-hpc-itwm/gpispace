#ifndef FHGLOG_CONFIGURATION_HPP
#define FHGLOG_CONFIGURATION_HPP 1

#include <fhglog/fhglog-config.hpp>
#include <fhglog/fhglog.hpp>
#include <fhglog/CompoundAppender.hpp>
#include <fhglog/ThreadedAppender.hpp>
#include <fhglog/util.hpp> // environment parser

#include <iostream>

namespace fhg { namespace log {
  class DefaultConfiguration {
    public:
    static const std::string & STDOUT() { static std::string s("stdout"); return s; }
    static const std::string & STDERR() { static std::string s("stderr"); return s; }
    static const std::string & STDLOG() { static std::string s("stdlog"); return s; }

      DefaultConfiguration()
        : level_(LogLevel::DEF_LEVEL)
        , to_console_("")
        , to_file_("")
        , to_server_("")
        , fmt_string_("")
          // FIXME: broken if set to true
        , threaded_(false)
        , color_("auto")
      {}

      void operator() () throw() {
#if FHGLOG_DISABLE_LOGGING != 1
#ifndef NDEBUG_FHGLOG
        std::clog << "I: performing default logging configuration" << std::endl;
#endif
        try {
          getLogger().removeAllAppenders();

          parse_environment();
          if (! check_config())
          {
            fallback_configuration();
          }

          configure();
        } catch (const std::exception& ex) {
          std::clog << "E: Could not configure the logging environment: " << ex.what() << std::endl;
        } catch (...) {
          std::clog << "E: Could not configure the logging environment: " << "unknown error" << std::endl;
        }
        std::clog.flush();
#endif
      }

    private:
      /*
       * This configuration takes the following environment variables into account (case sensitive):
       *  FHGLOG_level                      // log everything with at least this level, defaults to TRACE
       *  FHGLOG_format=log-format          // defaults to the default format in Formatter
       *  FHGLOG_to_console={stdout,stderr,stdlog} // log to stdout, stderr, clog
       *  FHGLOG_to_file=path to logfile    // log to specified file
       *  FHGLOG_to_server=ip:port          // log to the specified server
       */
      void parse_environment()
      {
        environment_t env = get_environment_variables();

        for (environment_t::const_iterator entry(env.begin()); entry != env.end(); ++entry)
        {
          std::string key(entry->first);
          std::string val(entry->second);
          parse_key_value(key, val);
        }
      }

      bool check_config()
      {
        if (to_console_.empty()
         && to_server_.empty()
         && to_file_.empty()
        )
        {
          return false;
        }

        return true;
      }

      void configure()
      {
        std::string fmt (default_format::SHORT());

        if (fmt_string_.size())
        {
#ifndef NDEBUG
          std::clog << "D: setting format to \"" << fmt_string_ << "\"" << std::endl;
#endif
          if      (fmt_string_ == "full")    fmt = default_format::LONG();
          else if (fmt_string_ == "short")   fmt = default_format::SHORT();
          else if (fmt_string_ == "default") fmt = default_format::SHORT();
          else                               fmt = fmt_string_;

          check_format (fmt);
        }

	CompoundAppender::ptr_t compound_appender(new CompoundAppender("auto-config-appender"));

	if (STDERR() == to_console_)
	{
	  compound_appender->addAppender
            (Appender::ptr_t(new StreamAppender( "console"
                                               , std::cerr
                                               , fmt
                                               , color_ != "no"
                                               )
                            )
            );
#ifndef NDEBUG_FHGLOG
	  std::clog << "D: logging to console: " << to_console_ << std::endl;
#endif
	}
	else if (STDOUT() == to_console_)
	{
	  compound_appender->addAppender
            (Appender::ptr_t(new StreamAppender( "console"
                                               , std::cout
                                               , fmt
                                               , color_ != "no"
                                               )
                            )
            );
#ifndef NDEBUG_FHGLOG
	  std::clog << "D: logging to console: " << to_console_ << std::endl;
#endif
	}
	else if (STDLOG() == to_console_)
	{
	  compound_appender->addAppender
            (Appender::ptr_t(new StreamAppender( "console"
                                               , std::clog
                                               , fmt
                                               , color_ != "no"
                                               )
                            )
            );
#ifndef NDEBUG_FHGLOG
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
                                               , color_ != "no"
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
                                               , default_format::LONG()
                                               )
                              )
              );
#ifndef NDEBUG_FHGLOG
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
#ifndef NDEBUG_FHGLOG
            std::clog << "D: logging to server: " << to_server_ << std::endl;
#endif
          }
          catch (const std::exception &ex)
          {
            std::clog << "E: could not create remote logger to: " << to_server_ << ": " << ex.what() << std::endl;
          }
        }
#endif

#ifndef NDEBUG_FHGLOG
        std::clog << "D: loglevel set to " << level_ << std::endl;
#endif
        getLogger().setLevel(level_);
		if (threaded_)
		{
		  getLogger().addAppender(Appender::ptr_t(new ThreadedAppender(compound_appender)));
		}
		else
		{
		  getLogger().addAppender(compound_appender);
		}
      }

      void fallback_configuration()
      {
        to_console_ = "stderr";
        to_file_ = "";
        to_server_ = "";
      }

      void parse_key_value(const std::string &key, const std::string &val)
      {
        if (key == "FHGLOG_level")
        {
          level_ = LogLevel(val);
        }
        else if (key == "FHGLOG_format")
        {
          fmt_string_ = val;
        }
        else if (key == "FHGLOG_to_console")
        {
          to_console_ = val;
        }
        else if (key == "FHGLOG_to_file")
        {
          to_file_ = val;
        }
        else if (key == "FHGLOG_to_server")
        {
          to_server_ = val;
        }
        else if (key == "FHGLOG_threaded" && (val == "no" || val == "false" || val == "0"))
        {
          threaded_ = false;
        }
        else if (key == "FHGLOG_color" && (val == "auto" || val == "no" || val == "yes"))
        {
          color_ = val;
        }
        else if (key.substr(0, 6) == "FHGLOG")
        {
#ifndef NDEBUG_FHGLOG
        std::clog << "D: ignoring key: " << key << std::endl;
#endif
        }
        else
        {
          // completely ignore
        }
      }

    private:
      // internal config variables
      fhg::log::LogLevel level_;
      std::string to_console_;
      std::string to_file_;
      std::string to_server_;
      std::string fmt_string_;
      bool threaded_;
      std::string color_;
  };

  class Configurator {
    public:
      static void configure() {
        configure(DefaultConfiguration());
      }
      static void configure(int , char *[]) {
        // parameters currently ignored
        configure();
      }
      template<class CF> static void configure(CF cf) {
        (cf)();
      }
  };

  template <typename Fun>
  inline
  void configure (Fun fun)
  {
    fun();
  }

  inline
  void configure ()
  {
    Configurator::configure();
  }

  inline
  void configure (int ac, char *av[])
  {
    Configurator::configure(ac, av);
  }
}}

#endif
