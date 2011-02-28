#ifndef WINDOWAPPENDER_HPP
#define WINDOWAPPENDER_HPP

#include <fhglog/Appender.hpp>

#include <QApplication>

class MonitorWindow;

class WindowAppender : public fhg::log::Appender
{
public:
  explicit
  WindowAppender (MonitorWindow *win, int typ)
    : fhg::log::Appender ("gui")
    , m_win (win)
    , m_typ (typ)
 {}

  void append (fhg::log::LogEvent const &evt)
  {
    QApplication::postEvent (m_win, new LogEventWrapper(m_typ, evt));
  }

  void flush()
  {}
private:
  MonitorWindow *m_win;
  int m_typ;
};

#endif // WINDOWAPPENDER_HPP
