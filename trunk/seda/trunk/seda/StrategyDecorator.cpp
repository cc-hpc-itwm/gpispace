#include "StrategyDecorator.hpp"

using namespace seda;

StrategyDecorator::StrategyDecorator(const std::string& name, const seda::Strategy::Ptr& s)
  : Strategy(name), _component(s) {}

StrategyDecorator::~StrategyDecorator() {}

void StrategyDecorator::perform(const IEvent::Ptr& e) {
  _component->perform(e);
}
