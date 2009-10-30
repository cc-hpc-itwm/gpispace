#ifndef SEDA_STRATEGY_DECORATOR_HPP
#define SEDA_STRATEGY_DECORATOR_HPP 1

#include <seda/Strategy.hpp>

namespace seda {
  class StrategyDecorator : public seda::Strategy {
  public:
    StrategyDecorator(const std::string& name, const seda::Strategy::Ptr& s);
    virtual ~StrategyDecorator();
    virtual void perform(const IEvent::Ptr&);

    virtual void onStageStart(const std::string &s)
    {
      _component->onStageStart(s);
    }
    virtual void onStageStop(const std::string &s)
    {
      _component->onStageStop(s);
    }
  private:
    seda::Strategy::Ptr _component;
  };
}

#endif // !SEDA_STRATEGY_DECORATOR_HPP
