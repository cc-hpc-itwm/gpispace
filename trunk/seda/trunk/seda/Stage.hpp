#ifndef SEDA_STAGE_HPP
#define SEDA_STAGE_HPP 1

#include <string>
#include <list>

#include <boost/thread.hpp>

#include <seda/common.hpp>
#include <seda/shared_ptr.hpp>
#include <seda/constants.hpp>
#include <seda/SedaException.hpp>
#include <seda/StageNotFound.hpp>
#include <seda/IEvent.hpp>
#include <seda/IEventQueue.hpp>
#include <seda/Strategy.hpp>

namespace seda {
    class StageWorker;
    class Stage {
    private:
        typedef struct {
            boost::thread thread;
            StageWorker  *worker;
        } ThreadInfo;
        typedef std::list<ThreadInfo*> ThreadPool;
    public:
        typedef std::tr1::shared_ptr<Stage> Ptr;
    
        Stage(const std::string& name, Strategy::Ptr strategy, std::size_t maxPoolSize=1, std::size_t maxQueueSize=SEDA_MAX_QUEUE_SIZE, const std::string& errorHandler="system-event-handler");
        Stage(const std::string& name, IEventQueue::Ptr queue, Strategy::Ptr strategy, std::size_t maxPoolSize=1, const std::string& errorHandler="system-event-handler");
    
        virtual ~Stage();

        virtual void start();
        virtual void stop();

        virtual bool empty() const { return queue()->empty(); }
        virtual std::size_t size() const { return queue()->size(); }
        virtual void waitUntilEmpty() const { queue()->waitUntilEmpty(); }
        virtual void waitUntilEmpty(unsigned long millis) { queue()->waitUntilEmpty(millis); }
    
        virtual void waitUntilNonEmpty() const { queue()->waitUntilNotEmpty(); }
        virtual void waitUntilNonEmpty(unsigned long millis) { queue()->waitUntilNotEmpty(millis); }

        virtual const std::string& name() const { return _name; }
    
        virtual Strategy::Ptr strategy() { return _strategy; }
        virtual const Strategy::Ptr strategy() const { return _strategy; }

        static void send(const std::string& stageName, const IEvent::Ptr& e) throw (QueueFull, StageNotFound);
        
        virtual void send(const IEvent::Ptr& e) throw (QueueFull) {
            queue()->push(e);
        }
        virtual IEvent::Ptr recv(unsigned long millis) throw (QueueEmpty) {
            return queue()->pop(millis);
        }

        void setErrorHandler(const std::string& eh) { _error_handler = eh; }
        const std::string& getErrorHandler() const { return _error_handler; }
        unsigned long timeout() const { return _timeout; }
        void timeout(unsigned long millis) { _timeout = millis; }
    private:
        IEventQueue::Ptr queue() { return _queue; }
        const IEventQueue::Ptr queue() const { return _queue; }

        SEDA_DECLARE_LOGGER();
        IEventQueue::Ptr _queue;
        Strategy::Ptr _strategy;
        std::string _name;
        std::string _error_handler;
        std::size_t _maxPoolSize;
        unsigned long _timeout;
        ThreadPool _threadPool;
    };
}

#endif // !SEDA_STAGE_HPP
