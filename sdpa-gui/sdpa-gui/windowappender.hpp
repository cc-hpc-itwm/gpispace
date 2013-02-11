#ifndef WINDOWAPPENDER_HPP
#define WINDOWAPPENDER_HPP

#include <fhglog/Appender.hpp>
#include <boost/function.hpp>

class WindowAppender : public fhg::log::Appender
{
public:
  typedef boost::function<void (fhg::log::LogEvent const &)> event_handler_t;

  WindowAppender (const event_handler_t& handler)
    : fhg::log::Appender ("event-handler")
    , m_handler (handler)
 {}

  void append (fhg::log::LogEvent const &evt)
  {
    m_handler(evt);
  }

  void flush()
  {}
private:
  event_handler_t m_handler;
};

#endif // WINDOWAPPENDER_HPP
