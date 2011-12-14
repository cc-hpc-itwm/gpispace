#ifndef LOGEVENTWRAPPER_HPP
#define LOGEVENTWRAPPER_HPP

#include <fhglog/LogEvent.hpp>
#include <QEvent>

class LogEventWrapper : public QEvent
{
public:
    explicit
    LogEventWrapper(int typ, const fhg::log::LogEvent &);
    ~LogEventWrapper ();

    fhg::log::LogEvent log_event;
};

#endif // LOGEVENTWRAPPER_HPP
