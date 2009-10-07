#ifndef SEDA_FORWARD_STRATEGY_HPP
#define SEDA_FORWARD_STRATEGY_HPP 1

#include <seda/Strategy.hpp>

namespace seda {
    class ForwardStrategy : public Strategy {
    public:
        explicit
        ForwardStrategy(const std::string& next)
            : Strategy("fwd-to-"+next), _next(next) {}
        
        ForwardStrategy(const std::string& name, const std::string& next)
            : Strategy(name), _next(next) {}
        virtual ~ForwardStrategy() {}

        virtual void perform(const IEvent::Ptr&);

        virtual void next(const std::string& next) { _next = next; }
        virtual const std::string& next() const { return _next; }
    private:
        std::string _next;
    };
}

#endif // !SEDA_FORWARD_STRATEGY_HPP
