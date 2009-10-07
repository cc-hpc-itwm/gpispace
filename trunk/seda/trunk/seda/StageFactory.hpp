#ifndef SEDA_STAGE_FACTORY_HPP
#define SEDA_STAGE_FACTORY_HPP 1

#include <seda/Stage.hpp>
#include <seda/StageRegistry.hpp>

namespace seda {

    class StageFactory {
        public:
            typedef std::tr1::shared_ptr<StageFactory> Ptr;

            explicit
            StageFactory() {}

            virtual seda::Stage::Ptr createStage(const std::string &name,
                    seda::Strategy::Ptr strategy,
                    std::size_t maxPoolSize=1,
                    std::size_t maxQueueSize=SEDA_MAX_QUEUE_SIZE,
                    const std::string &errorHandler="system-event-handler") const {
                seda::Stage::Ptr stage(new seda::Stage(name, strategy, maxPoolSize, maxQueueSize, errorHandler));
                StageRegistry::instance().insert(stage);
                return stage;
            }
    };
}

#endif // SEDA_STAGE_FACTORY_HPP
