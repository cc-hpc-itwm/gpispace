#ifndef FHG_LOG_LOGEVENT_HPP
#define FHG_LOG_LOGEVENT_HPP 1

#include <string>

namespace fhg { namespace log {
  class LogEvent {
  public:
    typedef std::string file_type;
    typedef std::string function_type;
    typedef std::size_t line_type;
    typedef std::string message_type;
    typedef long long tstamp_type;
    typedef int thread_type;

    LogEvent(const file_type &file
           , const function_type &function
           , const line_type &line
           , const message_type &message);

    LogEvent(const LogEvent &);

    ~LogEvent();

    LogEvent &operator=(const LogEvent &);
    bool operator==(const LogEvent &);
    bool operator!=(const LogEvent &rhs)
    {
      return !(*this == rhs);
    }

    bool operator<(const LogEvent &);

    inline const file_type &file() const { return file_; }
    inline const function_type &function() const { return function_; }
    inline const line_type &line() const { return line_; }
    inline const message_type &message() const { return message_; }
    inline const tstamp_type &tstamp() const { return tstamp_; }
    inline const thread_type &thread() const { return thread_; }
  private:
    file_type file_;
    function_type function_;
    line_type line_;
    message_type message_;
    tstamp_type tstamp_;
    thread_type thread_;
  };
}}

#endif
