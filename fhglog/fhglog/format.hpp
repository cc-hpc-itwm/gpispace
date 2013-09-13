#ifndef FHG_LOG_FORMAT_HPP
#define FHG_LOG_FORMAT_HPP 1

#include <string>
#include <fhglog/LogEvent.hpp>
#include <fhglog/format_flag.hpp>

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
        if (*c == fhg::log::fmt::flag::OPEN::value)
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
#define HDL(f)                                                         \
            case fmt::format_traits<fmt::flag:: f>::flag::value:        \
              fmt::format_traits<fmt::flag:: f>::flag::format(evt, os, *c); \
            break

            HDL(OPEN);
            HDL(SHORT_SEVERITY);
            HDL(SEVERITY);
            HDL(FILE);
            HDL(PATH);
            HDL(FUNCTION);
            HDL(MODULE);
            HDL(LINE);
            HDL(MESSAGE);
            HDL(DATE);
            HDL(TSTAMP);
            HDL(TID);
            HDL(PID);
            HDL(TAGS);
            HDL(LOGGER);
            HDL(ENDL);
          default:
            HDL(UNDEF);
#undef HDL
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

    struct formatter
    {
      formatter (const std::string & s = default_format::SHORT())
        : fmt_(s)
      {}

      std::string operator () (const LogEvent & e) const
      {
        return format(fmt_, e);
      }
    private:
      std::string fmt_;
    };
  }
}

#endif
