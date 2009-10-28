#ifndef SEDA_DISCARD_STRATEGY_HPP
#define SEDA_DISCARD_STRATEGY_HPP 1

#include <iostream>

#include <seda/Strategy.hpp>

namespace seda {
    class DiscardStrategy : public Strategy {
        public:
            explicit
                DiscardStrategy(const std::string &a_name = "discard")
                : Strategy(a_name) {}
            ~DiscardStrategy() {}

            void perform(const IEvent::Ptr& e) {
                SEDA_LOG_DEBUG("discarding: " << e->str());
            }
    };
}

#endif // !SEDA_DISCARD_STRATEGY_HPP
