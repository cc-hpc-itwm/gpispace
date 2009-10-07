#ifndef SEDA_STAGEREGISTRY_HPP
#define SEDA_STAGEREGISTRY_HPP 1

#include <seda/Stage.hpp>
#include <seda/StageNotFound.hpp>
#include <seda/StageAlreadyRegistered.hpp>
#include <map>

namespace seda {
    /**
     * TODO: make this class thread-safe!
     */
    class StageRegistry {
    public:
        ~StageRegistry();

        static StageRegistry& instance();

        // access functions

        /**
         * Register a new stage with the supplied name.
         */
        void insert(const std::string& name, const Stage::Ptr& stage) throw(StageAlreadyRegistered);

        /**
         * Register a new stage and take over the ownership of it.
         */
        void insert(const std::string& name, Stage* stage) throw(StageAlreadyRegistered);

        /**
         * Register a new stage with the name of the stage.
         */
        void insert(const Stage::Ptr& stage) throw(StageAlreadyRegistered);

        /**
         * Register a new stage and take over the ownership of it.
         */
        void insert(Stage* stage) throw(StageAlreadyRegistered);
        
        /**
         * Lookup a stage by its name.
         */
        const Stage::Ptr lookup(const std::string& name) const throw(StageNotFound);

        /**
         * Remove a stage from the registry
         */
        bool remove(const Stage::Ptr &stage);

        /**
         * Remove a stage from the registry using its name.
         */
        bool remove(const std::string &name);

        /**
         * Collectively start all stages.
         */
        void startAll();

        /**
         * Collectively stop all stages.
         */
        void stopAll();

        /**
         * Remove all registered stages.
         */
        void clear();
    private:
        StageRegistry();
        StageRegistry(const StageRegistry&);
        void operator=(const StageRegistry&);

        SEDA_DECLARE_LOGGER();
        std::map<std::string, Stage::Ptr> _stages;
    };
}

#endif // ! SEDA_STAGEREGISTRY_HPP
