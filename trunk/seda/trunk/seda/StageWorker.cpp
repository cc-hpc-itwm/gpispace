#include "StageWorker.hpp"
#include "Stage.hpp"
#include "IEvent.hpp"
#include "EventNotSupported.hpp"
#include "SystemEvent.hpp"
#include "StageRegistry.hpp"

namespace seda {
    void StageWorker::run() {

        while (!stopped()) {
            try {
                IEvent::Ptr e = _stage->recv(_stage->timeout());
                _busy = true; SEDA_LOG_DEBUG("got work: " << e->str());

                try {
                    _stage->strategy()->perform(e);
                } catch (const seda::EventNotSupported&) {
                    Stage::Ptr systemEventHandler(StageRegistry::instance().lookup(_stage->getErrorHandler()));
                    if (systemEventHandler.get() != _stage) {
                        systemEventHandler->send(e);
                    } else {
                        SEDA_LOG_FATAL("received a SystemEvent, but it could not be handled!");
                    }
                }

                _busy = false; SEDA_LOG_DEBUG("done");
            } catch (const seda::QueueEmpty&) {
                // ignore
            } catch (const seda::QueueFull&) {
                SEDA_LOG_ERROR("event discarded due to overflow protection");
            } catch (const seda::StageNotFound& snf) {
                SEDA_LOG_ERROR("event not handled, stage `" << snf.stageName() << "' could not be found!");
            } catch (const std::exception& ex) {
                SEDA_LOG_ERROR("strategy execution failed: " << ex.what());
            } catch (...) {
                SEDA_LOG_ERROR("strategy execution failed: unknown reason");
            }
        }
        SEDA_LOG_DEBUG("terminating");
    }
}
