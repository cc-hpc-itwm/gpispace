#include "Logger.hpp"

using namespace fhg::log;

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

bool LoggerApi::isLevelEnabled(const LogLevel &level)
{
  return impl_->isLevelEnabled(level);
}
void LoggerApi::log(const LogEvent &event)
{
  impl_->log(event);
}
Appender::ptr_t LoggerApi::addAppender(Appender::ptr_t appender)
{
  return impl_->addAppender(appender);
}
Appender::ptr_t LoggerApi::getAppender(const std::string &appender_name)
{
  return impl_->getAppender(appender_name);
}
void LoggerApi::removeAppender(const std::string &appender_name)
{
  return impl_->removeAppender(appender_name);
}


