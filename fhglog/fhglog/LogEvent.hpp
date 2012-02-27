#ifndef FHG_LOG_LOGEVENT_HPP
#define FHG_LOG_LOGEVENT_HPP 1

#include <string>
#include <sstream>
#include <sys/types.h>
#include <fhglog/LogLevel.hpp>

// serialization
#include <boost/serialization/string.hpp>
#include <boost/serialization/nvp.hpp>

namespace fhg { namespace log {
  class LogEvent {
    public:
      typedef LogLevel severity_type;
      typedef std::string file_type;
      typedef std::string function_type;
      typedef std::size_t line_type;
      typedef std::string message_type;
      typedef std::string module_type;
      typedef unsigned long long tstamp_type;
      typedef pid_t         pid_type;
      typedef unsigned long tid_type;

      LogEvent();
      LogEvent(const severity_type &severity
             , const file_type &path
             , const function_type &function
             , const line_type &line
             , const message_type &message = "");

      LogEvent(const LogEvent &);

      ~LogEvent();

      LogEvent &operator=(const LogEvent &);
      bool operator==(const LogEvent &) const;
      bool operator!=(const LogEvent &rhs) const
      {
        return !(*this == rhs);
      }

      bool operator<(const LogEvent &) const;

      inline const severity_type &severity() const { return severity_; }
      inline const file_type &file() const { return file_; }
      inline const file_type &path() const { return path_; }
      inline const function_type &function() const { return function_; }
      inline const line_type &line() const { return line_; }
      inline const message_type &message() const { return message_; }
      inline const tstamp_type &tstamp() const { return tstamp_; }
      inline const pid_type &pid() const { return pid_; }
      inline const tid_type &tid() const { return tid_; }
      inline const std::string &logged_via() const { return logged_via_; }
      inline const std::string &logged_on() const { return logged_on_; }
      inline const std::string &module() const { return module_; }

      inline severity_type &severity() { return severity_; }
      inline file_type &file() { return file_; }
      inline file_type &path() { return path_; }
      inline function_type &function() { return function_; }
      inline line_type &line() { return line_; }
      inline message_type &message() { return message_; }
      inline tstamp_type &tstamp() { return tstamp_; }
      inline pid_type &pid() { return pid_; }
      inline tid_type &tid() { return tid_; }
      inline std::string &logged_via() { return logged_via_; }
      inline std::string &logged_on() { return logged_on_; }
      inline std::string &module() { return module_; }

      inline void logged_via(const std::string &name) const
      {
        const_cast<std::string&>(logged_via_) = name;
      }

      inline void finish() const
      {
        if (message_.empty())
        {
          const_cast<std::string&>(message_) = message_buffer_.str();
        }
      }
      inline std::ostream &stream() { return message_buffer_; }

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int /* version */ )
    {
      ar & BOOST_SERIALIZATION_NVP( severity_ );
      ar & BOOST_SERIALIZATION_NVP( path_ );
      ar & BOOST_SERIALIZATION_NVP( file_ );
      ar & BOOST_SERIALIZATION_NVP( function_ );
      ar & BOOST_SERIALIZATION_NVP( line_ );
      ar & BOOST_SERIALIZATION_NVP( message_ );
      ar & BOOST_SERIALIZATION_NVP( tstamp_ );
      ar & BOOST_SERIALIZATION_NVP( pid_ );
      ar & BOOST_SERIALIZATION_NVP( tid_ );
      ar & BOOST_SERIALIZATION_NVP( logged_via_ );
      ar & BOOST_SERIALIZATION_NVP( logged_on_ );
      ar & BOOST_SERIALIZATION_NVP( module_ );
    }

    private:
      severity_type severity_;
      file_type path_;
      file_type file_;
      function_type function_;
      line_type line_;
      message_type message_;
      tstamp_type tstamp_;
      pid_type pid_;
      tid_type tid_;
      std::string logged_via_;
      std::string logged_on_;
      std::string module_;
      std::ostringstream message_buffer_;
  };
}}

#endif
