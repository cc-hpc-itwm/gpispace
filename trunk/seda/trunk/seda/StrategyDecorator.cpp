#include "StrategyDecorator.hpp"

using namespace seda;

StrategyDecorator::StrategyDecorator(const std::string& a_name, const seda::Strategy::Ptr& s)
  : Strategy(a_name), _component(s) {}

StrategyDecorator::~StrategyDecorator() {}

void StrategyDecorator::perform(const IEvent::Ptr& e) {
  _component->perform(e);
}
