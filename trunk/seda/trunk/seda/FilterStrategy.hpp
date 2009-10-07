#ifndef SEDA_FILTER_STRATEGY_HPP
#define SEDA_FILTER_STRATEGY_HPP 1

#include <seda/StrategyDecorator.hpp>

namespace seda {

    template <class C>
    class FilterStrategy : public StrategyDecorator {
        public:
            explicit
                FilterStrategy(const Strategy::Ptr& s)
                : StrategyDecorator(s->name()+".filter", s)
                {}
            ~FilterStrategy() {}

            void perform(const IEvent::Ptr& e) {
                if (dynamic_cast<C*>(e.get()) != NULL) {
                    SEDA_LOG_DEBUG("filtering: " << e->str());
                } else {
                    StrategyDecorator::perform(e);
                }
            }
    };
}

#endif // !SEDA_FILTER_STRATEGY_HPP
