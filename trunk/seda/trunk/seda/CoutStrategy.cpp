#include "IEvent.hpp"
#include "IEventQueue.hpp"
#include "CoutStrategy.hpp"

namespace seda {
    void CoutStrategy::perform(const IEvent::Ptr& e) {
        _os << e->str() << std::endl;
    }
}
