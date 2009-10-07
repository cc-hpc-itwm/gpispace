#include "LossyDaemonStrategy.hpp"

#include <limits>

using namespace seda;

LossyDaemonStrategy::LossyDaemonStrategy(const Strategy::Ptr &s, double probability, unsigned int seed)
  : StrategyDecorator(s->name()+".lossy-daemon", s),
    probability_(probability),
    random_(generator_type(seed))
{
}

void
LossyDaemonStrategy::perform(const IEvent::Ptr &evt) {
    if ( random_() <= probability_) {
        SEDA_LOG_DEBUG("lossy-daemon lost event: " << evt->str());
    } else {
        StrategyDecorator::perform(evt);
    } 
}
