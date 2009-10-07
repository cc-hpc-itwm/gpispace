#ifndef SEDA_IEVENT_HPP
#define SEDA_IEVENT_HPP

#include <string>
#include <ostream>
#include <seda/shared_ptr.hpp>

namespace seda {
    class IEvent {
    public:
        typedef std::tr1::shared_ptr<IEvent> Ptr;
    
        virtual ~IEvent() {}
        virtual std::string str() const = 0;
        std::ostream& operator<<(std::ostream& os) {
            return os << str();
        }

        /* TODO: introduce timeouts to preserve the ordering of events? */
    protected:
        IEvent() {}
    };
}

#endif // !SEDA_IEVENT_HPP
