#include <fhglog/event.hpp>

#include <fhg/util/macros.hpp>
#include <fhg/util/num.hpp>
#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/require.hpp>
#include <util-generic/hostname.hpp>
#include <fhg/util/now.hpp>

#include <iostream>
#include <sstream>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/syscall.h>

namespace fhg
{
  namespace log
  {
    LogEvent::LogEvent( const Level &a_severity
                      , const std::string &a_message
                      )
      : severity_(a_severity)
      , message_(a_message)
      , tstamp_(fhg::util::now())
      , pid_(getpid())
      , tid_(syscall (SYS_gettid))
      , host_ (fhg::util::hostname())
    {}

    LogEvent::LogEvent()
      : severity_ (INFO)
      , message_()
      , tstamp_()
      , pid_()
      , tid_()
      , host_ ()
    {
    }

    bool LogEvent::operator< (const LogEvent &rhs) const
    {
      return tstamp() < rhs.tstamp();
    }

    std::string LogEvent::encoded() const
    {
      std::ostringstream os;
      os << *this;
      return os.str();
    }

    namespace
    {
      template<typename T>
        T read_integral (fhg::util::parse::position& pos)
      {
        T x (0);
        while (!pos.end() && isdigit (*pos))
        {
          x *= 10;
          x += *pos - '0';
          ++pos;
        }
        return x;
      }
      std::string read_string (fhg::util::parse::position& pos)
      {
        unsigned long s (read_integral<unsigned long> (pos));
        ++pos;
        std::string accum; accum.reserve (s);
        for (; s > 0; --s, ++pos)
        {
          accum.push_back (*pos);
        }
        return accum;
      }
      Level read_loglevel (fhg::util::parse::position& pos)
      {
        switch (*pos)
        {
        case 'T': ++pos; return TRACE;
        case 'I': ++pos; return INFO;
        case 'W': ++pos; return WARN;
        case 'E': ++pos; return ERROR;
        }

        throw std::runtime_error ("unknown log level");
      }
    }

    LogEvent::LogEvent (fhg::util::parse::position& pos)
      : severity_ ((++pos, read_loglevel (pos)))
      , message_ ((++pos, read_string (pos)))
      , tstamp_ ((++pos, fhg::util::read_double (pos)))
      , pid_ ((++pos, read_integral<pid_t> (pos)))
      , tid_ ((++pos, read_integral<pid_t> (pos)))
      , host_ ((++pos, read_string (pos)))
    {}

    LogEvent LogEvent::from_string (const std::string& str)
    {
      fhg::util::parse::position_string pos (str);
      return LogEvent (pos);
    }
  }
}

namespace
{
  namespace encode
  {
    class string
    {
    public:
      string (std::string const& s)
        : _s (s)
      {}
      std::ostream& operator() (std::ostream& os) const
      {
        return os << _s.size() << '_' << _s;
      }
    private:
      std::string const& _s;
    };
    std::ostream& operator<< (std::ostream& os, string const& s)
    {
      return s (os);
    }

    char loglevel (fhg::log::Level const& level)
    {
      switch (level)
      {
      case fhg::log::TRACE: return 'T';
      case fhg::log::INFO: return 'I';
      case fhg::log::WARN: return 'W';
      case fhg::log::ERROR: return 'E';
      }

      INVALID_ENUM_VALUE (fhg::log::Level, level);
    }

    class tstamp
    {
    public:
      explicit
      tstamp (double const &t)
        : _t (t)
      {}
      std::ostream& operator() (std::ostream& os) const
      {
        std::ios_base::fmtflags flags(os.flags());
        os.unsetf (std::ios_base::scientific);
        os.setf (std::ios_base::fixed);
        os << _t;
        os.flags(flags);

        return os;
      }
    private:
      double const& _t;
    };
    std::ostream& operator<< (std::ostream& os, tstamp const& t)
    {
      return t (os);
    }
  }
}

std::ostream& operator<< (std::ostream& os, const fhg::log::LogEvent& event)
{
  os << ',' << encode::loglevel (event.severity());
  os << ',' << encode::string (event.message());
  os << ',' << encode::tstamp (event.tstamp());
  os << ',' << event.pid();
  os << ',' << event.tid();
  os << ',' << encode::string (event.host());

  return os;
}
