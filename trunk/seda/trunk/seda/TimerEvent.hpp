#ifndef SEDA_TIMER_EVENT_HPP
#define SEDA_TIMER_EVENT_HPP

#include <seda/SystemEvent.hpp>

namespace seda {
    /** */
    class TimerEvent : public SystemEvent {
    public:
        typedef std::tr1::shared_ptr<TimerEvent> Ptr;

        explicit
        TimerEvent(const std::string& tag) : _tag(tag) {}

        const std::string& tag() const { return _tag; }
        std::string str() const { return "timer-event."+tag(); }
    
        virtual ~TimerEvent() {}
    private:
        std::string _tag;
    };
}

#endif // !SEDA_UUSER_EVENT_HPP
