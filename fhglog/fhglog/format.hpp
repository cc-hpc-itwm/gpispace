#pragma once

#include <fhglog/event.hpp>
#include <fhglog/level_io.hpp>

#include <fhg/util/save_stream_flags.hpp>
#include <fhg/util/parse/position.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <string>

namespace fhg
{
  namespace log
  {
    inline
    std::ostream & format( std::ostream & os
                         , const std::string & format
                         , const LogEvent &evt
                         )
    {
      fhg::util::parse::position_string pos (format);

      while (!pos.end())
      {
        if (*pos != '%')
        {
          os << *pos;
          ++pos;
        }
        else
        {
          ++pos;

          if (pos.end())
          {
            throw std::runtime_error
              ("Format string ends with a single opening character!");
          }

          switch (*pos)
          {
          case '%': os << '%'; break;
          case 's': os << string (evt.severity())[0]; break;
          case 'S': os << string (evt.severity()); break;
          case 'p': os << boost::filesystem::path (evt.path()).filename().string(); break;
          case 'P': os << evt.path(); break;
          case 'F': os << evt.function(); break;
          case 'M': os << boost::filesystem::path (evt.path()).stem().string(); break;
          case 'L': os << evt.line(); break;
          case 'm': os << evt.message(); break;
          case 'R': os << evt.pid(); break;
          case 'n': os << "\n"; break;
          case 'd':
            {
              char buf[128];

              memset (buf, 0, sizeof (buf));

              const time_t tm (static_cast<time_t> (evt.tstamp()));

              ctime_r (&tm, buf);

              buf[std::find (buf, buf + sizeof (buf), '\n') - buf] = 0;
              os << buf;
            }
            break;
          case 't':
            {
              fhg::util::save_stream_flags const _ (os);
              os.unsetf (std::ios_base::scientific);
              os.setf (std::ios_base::fixed);
              os << evt.tstamp();
            }
            break;
          case 'T':
            {
              fhg::util::save_stream_flags const _ (os);
              os << std::hex << evt.tid();
            }
            break;
          default:
            throw std::runtime_error
              ((boost::format ("format code not defined: %1%") % *pos).str());
          }

          ++pos;
        }
      }

      return os;
    }

    inline
    std::string format(const std::string & fmt, const LogEvent &evt)
    {
      std::ostringstream os;
      format (os, fmt, evt);
      return os.str();
    }

    inline
    std::string check_format(const std::string & fmt)
    {
      std::ostringstream os;
      format (os, fmt, LogEvent());
      return fmt;
    }

    struct default_format
    {
      static std::string const & SHORT()
      {
        static std::string f("[%t] %s: %p:%L - %m%n");
        return f;
      }
      static std::string const & LONG()
      {
        static std::string f("%t %S pid:%R thread:%T %p:%L (%F) - %m%n");
        return f;
      }
    };
  }
}
