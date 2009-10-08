#ifndef SEDA_FORWARD_STRATEGY_HPP
#define SEDA_FORWARD_STRATEGY_HPP 1

#include <seda/Strategy.hpp>

namespace seda {
    class ForwardStrategy : public Strategy {
    public:
        explicit
        ForwardStrategy(const std::string& a_next)
            : Strategy("fwd-to-"+a_next), _next(a_next) {}
        
        ForwardStrategy(const std::string& a_name, const std::string& a_next)
            : Strategy(a_name), _next(a_next) {}
        virtual ~ForwardStrategy() {}

        virtual void perform(const IEvent::Ptr&);

        virtual void next(const std::string& a_next) { _next = a_next; }
        virtual const std::string& next() const { return _next; }
    private:
        std::string _next;
    };
}

#endif // !SEDA_FORWARD_STRATEGY_HPP
