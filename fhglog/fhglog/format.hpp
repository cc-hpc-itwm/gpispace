#ifndef FHG_LOG_FORMAT_HPP
#define FHG_LOG_FORMAT_HPP 1

#include <fhglog/event.hpp>

#include <fhg/util/first_then.hpp>

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include <string>

namespace fhg
{
  namespace log
  {
    inline
    std::ostream & format( std::ostream & os
                         , const std::string &_fmt
                         , const LogEvent &evt
                         )
    {
      std::string::const_iterator c(_fmt.begin());
      std::string::size_type len(_fmt.size());
      while (len > 0)
      {
        if (*c == '%')
        {
          ++c; --len;
          if (0 == len)
          {
            throw std::runtime_error
              ("Format string ends with a single opening character!");
          }

          // what do we have
          switch (*c)
          {
          case '%': os << '%'; break;
          case 's': os << evt.severity().str()[0]; break;
          case 'S': os << evt.severity().str(); break;
          case 'p': os << boost::filesystem::path (evt.path()).filename().string(); break;
          case 'P': os << evt.path(); break;
          case 'F': os << evt.function(); break;
          case 'M': os << boost::filesystem::path (evt.path()).stem().string(); break;
          case 'L': os << evt.line(); break;
          case 'm': os << evt.message(); break;
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
              std::ios_base::fmtflags const flags (os.flags());
              os.unsetf (std::ios_base::scientific);
              os.setf (std::ios_base::fixed);
              os << evt.tstamp();
              os.flags (flags);
            }
            break;
          case 'T':
            {
              std::ios_base::fmtflags const flags (os.flags());
              os << std::hex << evt.tid();
              os.flags (flags);
            }
            break;
          case 'R': os << evt.pid(); break;
          case '#':
            {
              fhg::util::first_then<std::string> sep ("#", ",#");

              BOOST_FOREACH (std::string const& tag, evt.tags())
              {
                os << sep << tag;
              }
            }
            break;
          case 'l':
            {
              fhg::util::first_then<std::string> sep ("", "->");

              BOOST_FOREACH (std::string const& t, evt.trace())
              {
                os << sep << t;
              }
            }
            break;
          case 'n': os << "\n"; break;
          default:
            throw std::runtime_error
              ((boost::format ("format code not defined: %1%") % *c).str());
          }
        }
        else
        {
          os << *c;
        }

        if (len)
        {
          ++c; --len;
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
    void check_format(const std::string & fmt)
    {
      std::ostringstream os;
      format (os, fmt, LogEvent());
    }

    struct default_format
    {
      static std::string const & SHORT()
      {
        static std::string f("[%t] %s: %l %p:%L - %m%n");
        return f;
      }
      static std::string const & LONG()
      {
        static std::string f("%t %S %l pid:%R thread:%T %p:%L (%F) - %m%n");
        return f;
      }
    };
  }
}

#endif
