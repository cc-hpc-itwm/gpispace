#ifndef SEDA_SYSTEM_EVENT_HPP
#define SEDA_SYSTEM_EVENT_HPP

#include <seda/IEvent.hpp>

namespace seda {
    /**
     * SystemEvents are special events  that are forwarded on reception to
     * a special-named stage.
     * 
     * The   special    stage   can   be   registered    with   the   name
     * "system-event-handler".   If  such   a  stage  is  registered,  all
     * system-events  are  passed to  that  stage,  if  no such  stage  is
     * registered, the event is logged as an fatal error.
     */
    class SystemEvent : public IEvent {
    public:
        typedef std::tr1::shared_ptr<SystemEvent> Ptr;
    
        virtual ~SystemEvent() {}
    protected:
        SystemEvent() {}
    };
}

#endif // !SEDA_SYSTEM_EVENT_HPP
