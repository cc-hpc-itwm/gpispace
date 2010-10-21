#ifndef FHG_LOG_FORMAT_FLAG_HPP
#define FHG_LOG_FORMAT_FLAG_HPP 1

#include <fhglog/LogEvent.hpp>
#include <ios>

namespace fhg
{
  namespace log
  {
    namespace fmt
    {
      namespace flag
      {
        struct OPEN
        {
          static const char value = '%';
          //        static const char *description = "starting character";
          static std::ostream & format( const LogEvent &
                                      , std::ostream &os
                                      , const char
                                      )
          {
            return os << value;
          }
        };

        struct SHORT_SEVERITY
        {
          static const char value = 's';
          //        static const char *description = "the level of the event";
          static std::ostream & format( const LogEvent &e
                                      , std::ostream &os
                                      , const char
                                      )
          {
            return os << e.severity().str()[0];
          }
        };

        struct SEVERITY
        {
          static const char value        = 'S';
          //        static const char *description = "the level of the event";
          static std::ostream & format( const LogEvent &e
                                      , std::ostream &os
                                      , const char
                                      )
          {
            return os << e.severity().str();
          }
        };

        struct FILE
        {
          static const char value            = 'p';
          //        static const char *description = "just the filename";
          static std::ostream & format( const LogEvent &e
                                      , std::ostream &os
                                      , const char
                                      )
          {
            return os << e.file();
          }
        };
        struct PATH
        {
          static const char value            = 'P';
          //        static const char *description = "complete path of the file";
          static std::ostream & format( const LogEvent &e
                                      , std::ostream &os
                                      , const char
                                      )
          {
            return os << e.path();
          }
        };
        struct FUNCTION
        {
          static const char value            = 'F';
          //        static const char *description = "function (full signature)";
          static std::ostream & format( const LogEvent &e
                                      , std::ostream &os
                                      , const char
                                      )
          {
            return os << e.function();
          }
        };
        struct MODULE
        {
          static const char value            = 'M';
          //        static const char *description = "module (i.e. filename without extension)";
          static std::ostream & format( const LogEvent &e
                                      , std::ostream &os
                                      , const char
                                      )
          {
            return os << e.module();
          }
        };
        struct LINE
        {
          static const char value            = 'L';
          //        static const char *description = "line of the log";
          static std::ostream & format( const LogEvent &e
                                      , std::ostream &os
                                      , const char
                                      )
          {
            return os << e.line();
          }
        };
        struct MESSAGE
        {
          static const char value            = 'm';
          //        static const char *description = "the logmessage";
          static std::ostream & format( const LogEvent &e
                                      , std::ostream &os
                                      , const char
                                      )
          {
            return os << e.message();
          }
        };
        struct DATE
        {
          static const char value            = 'd';
          //        static const char *description = "when was the event created (date string)";
          static std::ostream & format( const LogEvent &e
                                      , std::ostream &os
                                      , const char
                                      )
          {
            char buf[128];
            time_t tm = e.tstamp();
            ctime_r (&tm, buf);
            return os << buf;
          }
        };
        struct TSTAMP
        {
          static const char value            = 't';
          //        static const char *description = "when was the event created (unix timestamp)";
          static std::ostream & format( const LogEvent &e
                                      , std::ostream &os
                                      , const char
                                      )
          {
            return os << e.tstamp();
          }
        };
        struct TID
        {
          static const char value            = 'T';
          //        static const char *description = "by which thread has it been created";
          static std::ostream & format( const LogEvent &e
                                      , std::ostream &os
                                      , const char
                                      )
          {
            std::ios_base::fmtflags flags(os.flags());
            os << std::hex << e.tid();
            os.flags(flags);
            return os;
          }
        };
        struct PID
        {
          static const char value            = 'R';
          //        static const char *description = "by which process has it been created";
          static std::ostream & format( const LogEvent &e
                                      , std::ostream &os
                                      , const char
                                      )
          {
            return os << e.pid();
          }
        };
        struct LOGGER
        {
          static const char value            = 'l';
          //        static const char *description = "the logger via which this event came";
          static std::ostream & format( const LogEvent &e
                                      , std::ostream &os
                                      , const char
                                      )
          {
            return os << e.logged_via();
          }
        };
        struct ENDL
        {
          static const char value            = 'n';
          //        static const char *description = "new line";
          static std::ostream & format( const LogEvent &
                                      , std::ostream &os
                                      , const char
                                      )
          {
            return os << std::endl;
          }
        };
        struct UNDEF
        {
          static const char value            = '-';
          //        static const char *description = "placeholder for undefined flag";
          static std::ostream & format( const LogEvent &
                                      , std::ostream &
                                      , const char real
                                      )
          {
            std::ostringstream s;
            s << real;
            throw std::runtime_error ("format code not defined: " + s.str());
          }
        };
      }

      template <typename T = flag::UNDEF>
      struct format_traits
      {
        static const bool is_flag = false;
        typedef T flag;
      };

#define DECLARE_FLAG(cls)                             \
      template<>                                      \
      struct format_traits<flag::cls>                 \
      {                                               \
        static const bool is_flag = true;             \
        typedef flag:: cls flag;                      \
      }

      DECLARE_FLAG(OPEN);
      DECLARE_FLAG(SHORT_SEVERITY);
      DECLARE_FLAG(SEVERITY);
      DECLARE_FLAG(FILE);
      DECLARE_FLAG(PATH);
      DECLARE_FLAG(FUNCTION);
      DECLARE_FLAG(MODULE);
      DECLARE_FLAG(LINE);
      DECLARE_FLAG(MESSAGE);
      DECLARE_FLAG(DATE);
      DECLARE_FLAG(TSTAMP);
      DECLARE_FLAG(TID);
      DECLARE_FLAG(PID);
      DECLARE_FLAG(LOGGER);
      DECLARE_FLAG(ENDL);

#undef DECLARE_FLAG
    }
  }
}

#endif
