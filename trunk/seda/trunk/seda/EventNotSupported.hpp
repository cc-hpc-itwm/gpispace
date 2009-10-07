#ifndef SEDA_EVENT_NOT_SUPPORTED_HPP
#define SEDA_EVENT_NOT_SUPPORTED_HPP 1

#include <string>
#include <seda/SedaException.hpp>
#include <seda/IEvent.hpp>

namespace seda 
{
    class EventNotSupported : public SedaException {
    public:
        EventNotSupported(const seda::IEvent::Ptr &evt) :
            SedaException("event not supported"),
            _event(evt) {}
        virtual ~EventNotSupported() throw() { }
        
        virtual const seda::IEvent::Ptr &event() const { return _event; }
    private:
        seda::IEvent::Ptr _event;
    };
}

#endif // ! SEDA_EVENT_NOT_SUPPORTED
