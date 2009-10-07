#ifndef SEDA_LOGGING_STRATEGY_HPP
#define SEDA_LOGGING_STRATEGY_HPP 1

#include <seda/StrategyDecorator.hpp>

namespace seda {
  class LoggingStrategy : public StrategyDecorator {
  public:
    explicit
    LoggingStrategy(const Strategy::Ptr& s)
      : StrategyDecorator(s->name()+".logging", s)
    {}
    ~LoggingStrategy() {}

    void perform(const IEvent::Ptr& e) {
      SEDA_LOG_DEBUG(e->str());
      StrategyDecorator::perform(e);
    }
  };
}

#endif // !SEDA_LOGGING_STRATEGY_HPP
