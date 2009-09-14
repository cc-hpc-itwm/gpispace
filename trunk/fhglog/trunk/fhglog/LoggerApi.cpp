#include "LoggerApi.hpp"

using namespace fhg::log;

#if FHGLOG_ENABLED == 1
#include "Logger.hpp"

/*
 * LoggerApi implementation
 *
 */
void LoggerApi::setLevel(const LogLevel &level)
{
  impl_->setLevel(level);
}
const LogLevel &LoggerApi::getLevel() const
{
  return impl_->getLevel();
}

bool LoggerApi::isLevelEnabled(const LogLevel &level) const
{
  return impl_->isLevelEnabled(level);
}
void LoggerApi::log(const LogEvent &event) const
{
  impl_->log(event);
}
Appender::ptr_t LoggerApi::addAppender(Appender::ptr_t appender)
{
  return impl_->addAppender(appender);
}
Appender::ptr_t LoggerApi::getAppender(const std::string &appender_name) const
{
  return impl_->getAppender(appender_name);
}
void LoggerApi::removeAppender(const std::string &appender_name)
{
  return impl_->removeAppender(appender_name);
}

#endif
