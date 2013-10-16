#define BOOST_SPIRIT_THREADSAFE

#include "LogEvent.hpp"
#include "util.hpp"

#include <fhg/util/num.hpp>
#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/require.hpp>
#include <fhg/util/thread/atomic.hpp>

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>
#include <iterator>
#include <sstream>
#include <sys/time.h>

namespace fhg
{
  namespace log
  {
    namespace
    {
      LogEvent::tstamp_type now()
      {
        return time(NULL);
      }

      uint64_t next_id()
      {
        static fhg::thread::atomic<uint64_t> counter;
        return ++counter;
      }

      std::string get_hostname_ ()
      {
        char buf [4096];
        gethostname (buf, sizeof(buf));
        return buf;
      }

      std::string get_hostname ()
      {
        static std::string h (get_hostname_ ());
        return h;
      }
    }

    LogEvent::LogEvent( const severity_type &a_severity
                      , const file_type &a_path
                      , const function_type &a_function
                      , const line_type &a_line
                      , const std::string &a_message)
      : id_ (next_id ())
      , severity_(a_severity)
      , path_(a_path)
      , function_(a_function)
      , line_(a_line)
      , message_(a_message)
      , tstamp_(now())
      , pid_(getpid())
      , tid_(gettid())
      , host_ (get_hostname ())
      , trace_ ()
      , tags_ ()
    {}

    LogEvent::LogEvent()
      : id_ (next_id ())
      , severity_()
      , path_()
      , function_()
      , line_()
      , message_()
      , tstamp_()
      , pid_()
      , tid_()
      , host_ ()
      , trace_ ()
      , tags_ ()
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
      std::vector<std::string> read_vec (fhg::util::parse::position& pos)
      {
        std::vector<std::string> ret;

        while (!pos.end() && *pos != ',')
        {
          ret.push_back (read_string (pos));
        }

        return ret;
      }
      std::set<std::string> read_set (fhg::util::parse::position& pos)
      {
        std::set<std::string> ret;

        while (!pos.end())
        {
          ret.insert (read_string (pos));
        }

        return ret;
      }
      LogLevel::Level read_loglevel (fhg::util::parse::position& pos)
      {
        switch (*pos)
        {
        case 'T': ++pos; return LogLevel::TRACE;
        case 'D': ++pos; return LogLevel::DEBUG;
        case 'I': ++pos; return LogLevel::INFO;
        case 'W': ++pos; return LogLevel::WARN;
        case 'E': ++pos; return LogLevel::ERROR;
        case 'F': ++pos; return LogLevel::FATAL;
        }

        throw std::runtime_error ("unknown log level");
      }
    }

    LogEvent::LogEvent (fhg::util::parse::position& pos)
      : id_ ((++pos, read_integral<uint64_t> (pos)))
      , severity_ ((++pos, read_loglevel (pos)))
      , path_ ((++pos, read_string (pos)))
      , function_ ((++pos, read_string (pos)))
      , line_ ((++pos, read_integral<line_type> (pos)))
      , message_ ((++pos, read_string (pos)))
      , tstamp_ ((++pos, read_integral<tstamp_type> (pos)))
      , pid_ ((++pos, read_integral<pid_t> (pos)))
      , tid_ ((++pos, read_integral<unsigned long> (pos)))
      , host_ ((++pos, read_string (pos)))
      , trace_ ((++pos, read_vec (pos)))
      , tags_ ((++pos, read_set (pos)))
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

    char loglevel (fhg::log::LogLevel::Level const& level)
    {
      switch (level)
      {
      case fhg::log::LogLevel::TRACE: return 'T';
      case fhg::log::LogLevel::DEBUG: return 'D';
      case fhg::log::LogLevel::INFO: return 'I';
      case fhg::log::LogLevel::WARN: return 'W';
      case fhg::log::LogLevel::ERROR: return 'E';
      case fhg::log::LogLevel::FATAL: return 'F';
      }

      throw std::runtime_error ("unknown log level");
    }
  }
}

std::ostream& operator<< (std::ostream& os, const fhg::log::LogEvent& event)
{
  os << ',' << event.id();
  os << ',' << encode::loglevel (event.severity());
  os << ',' << encode::string (event.path());
  os << ',' << encode::string (event.function());
  os << ',' << event.line();
  os << ',' << encode::string (event.message());
  os << ',' << event.tstamp();
  os << ',' << event.pid();
  os << ',' << event.tid();
  os << ',' << encode::string (event.host());
  os << ',';
  BOOST_FOREACH (std::string const& t, event.trace())
  {
    os << encode::string (t);
  }
  os << ',';
  BOOST_FOREACH (std::string const& t, event.tags())
  {
    os << encode::string (t);
  }

  return os;
}
