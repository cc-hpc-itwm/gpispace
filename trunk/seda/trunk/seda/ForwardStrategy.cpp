#include "ForwardStrategy.hpp"

#include "IEvent.hpp"
#include "IEventQueue.hpp"
#include "StageRegistry.hpp"
#include <iostream>

namespace seda {
    void ForwardStrategy::perform(const IEvent::Ptr& e) {
        const Stage::Ptr& nextStage(StageRegistry::instance().lookup(_next));
        if (nextStage) {
            nextStage->send(e);
        } else {
            SEDA_LOG_WARN("nothing to forward to!");
        }
    }
}
