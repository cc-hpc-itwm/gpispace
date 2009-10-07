#include "Stage.hpp"
#include "IEvent.hpp"
#include "StageWorker.hpp"
#include "EventQueue.hpp"
#include "StageRegistry.hpp"

namespace seda {
    Stage::Stage(const std::string& name, Strategy::Ptr strategy, std::size_t maxPoolSize, std::size_t maxQueueSize, const std::string& errorHandler)
        : SEDA_INIT_LOGGER("seda.stage."+name),
          _queue(new EventQueue("seda.stage."+name+".queue", maxQueueSize)),
          _strategy(strategy),
          _name(name),
          _error_handler(errorHandler),
          _maxPoolSize(maxPoolSize),
          _timeout(SEDA_DEFAULT_TIMEOUT)
    {
    }

    Stage::Stage(const std::string& name, IEventQueue::Ptr queue, Strategy::Ptr strategy, std::size_t maxPoolSize, const std::string& errorHandler)
        : SEDA_INIT_LOGGER("seda.stage."+name),
          _queue(queue),
          _strategy(strategy),
          _name(name),
          _error_handler(errorHandler),
          _maxPoolSize(maxPoolSize),
          _timeout(SEDA_DEFAULT_TIMEOUT)
    {
    }
  
    Stage::~Stage() {
        try {
            // stop the running threads and delete them
            stop();
        } catch (const std::exception& e) {
            SEDA_LOG_ERROR("stopping failed: " << e.what());
        } catch (...) {
            SEDA_LOG_ERROR("stopping failed: unknown reason");
        }

        try {
            /* log input queue if not empty */
            if (!queue()->empty()) {
                SEDA_LOG_DEBUG("cleaning up input queue");
                while (!queue()->empty()) {
                    IEvent::Ptr e(queue()->pop());
                    SEDA_LOG_DEBUG("removed incoming event: " << e->str());
                }
                SEDA_LOG_DEBUG("done");
            }
        } catch (...) {}
    }

    void
    Stage::start() {
        if (_threadPool.empty()) {
            _strategy->onStageStart(name());

            // initialize and start worker threads
            for (std::size_t tId = 0; tId < _maxPoolSize; ++tId) {
                std::ostringstream sstr;
                sstr << "seda.stage." << name() << ".worker." << tId;
                ThreadInfo *i = new ThreadInfo;
                i->worker = new seda::StageWorker(sstr.str(), this);
                i->thread = boost::thread(boost::ref(*i->worker));
                _threadPool.push_back(i);
            }
        } // else == noop
    }

    void
    Stage::stop() {
        if (_threadPool.empty()) {
            return;
        }

        for (ThreadPool::iterator it(_threadPool.begin()); it != _threadPool.end(); ++it) {
            (*it)->worker->stop(); // signal threads to stop
        }
        queue()->wakeUpAll(); // release blocked threads

        while (!_threadPool.empty()) {
            ThreadInfo *i(_threadPool.front()); _threadPool.pop_front();
            i->thread.join();
            delete i->worker;
            delete i;
        }            
        _strategy->onStageStop(name());
    }

    void
    Stage::send(const std::string& stageName, const seda::IEvent::Ptr& e) throw (QueueFull, StageNotFound) {
        StageRegistry::instance().lookup(stageName)->send(e);
    }
}
