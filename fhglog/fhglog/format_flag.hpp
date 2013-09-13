#ifndef FHG_LOG_FORMAT_FLAG_HPP
#define FHG_LOG_FORMAT_FLAG_HPP 1

#include <fhglog/LogEvent.hpp>
#include <fhglog/util.hpp>
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <cstring>
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
            return os << get_filename_from_path (e.path ());
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
            return os << get_module_name_from_path (e.path ());
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

        inline
        bool isnewline (const char c)
        {
          return c == '\n' || c == '\r';
        }

        struct DATE
        {
          static const char value            = 'd';
          //static const char *description = "when was the event created (date string)";
          static std::ostream & format( const LogEvent &e
                                      , std::ostream &os
                                      , const char
                                      )
          {
            char buf[128]; memset (buf, 0, sizeof(buf));
            time_t tm = (e.tstamp()); // / 1000 / 1000);
            ctime_r (&tm, buf);
            std::string tmp (buf);
            boost::trim (tmp);
            return os << tmp;
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
        struct TAGS
        {
          static const char value            = '#';
          //        static const char *description = "the logger via which this event came";
          static std::ostream & format( const LogEvent &e
                                      , std::ostream &os
                                      , const char
                                      )
          {
            LogEvent::tags_type::const_iterator it = e.tags ().begin ();
            LogEvent::tags_type::const_iterator end = e.tags ().end ();
            if (it != end)
            {
              os << "#" << *it;
              ++it;
            }
            while (it != end)
            {
              os << "," << "#" << *it;
              ++it;
            }

            return os;
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
            std::vector<std::string>::const_iterator it = e.trace ().begin ();
            std::vector<std::string>::const_iterator end = e.trace ().end ();
            if (it != end)
            {
              os << *it;
              ++it;
            }
            while (it != end)
            {
              os << "->" << *it;
              ++it;
            }

            return os;
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
      DECLARE_FLAG(TAGS);
      DECLARE_FLAG(LOGGER);
      DECLARE_FLAG(ENDL);

#undef DECLARE_FLAG
    }
  }
}

#endif
