#ifndef SEDA_USER_EVENT_HPP
#define SEDA_USER_EVENT_HPP

#include <seda/IEvent.hpp>

namespace seda {
    /**
     * UserEvents are normal events that an application defines.
     *
     * Strategies are allowed to throw away or perform anything with those
     * events.
     */
    class UserEvent : public IEvent {
    public:
        typedef std::tr1::shared_ptr<UserEvent> Ptr;
    
        virtual ~UserEvent() {}
    protected:
        UserEvent() {}
    };
}

#endif // !SEDA_UUSER_EVENT_HPP
