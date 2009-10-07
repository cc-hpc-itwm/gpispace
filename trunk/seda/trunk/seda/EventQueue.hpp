#ifndef SEDA_EVENT_QUEUE_HPP
#define SEDA_EVENT_QUEUE_HPP 1

#include <list>
#include <assert.h>

#include <boost/thread.hpp>

#include <seda/common.hpp>
#include <seda/shared_ptr.hpp>
#include <seda/constants.hpp>
#include <seda/IEventQueue.hpp>
#include <seda/IEvent.hpp>

#define WAIT_INFINITE 0xFFFFffff

namespace seda {
    class EventQueue : public IEventQueue {
        public:
            typedef std::tr1::shared_ptr< EventQueue > Ptr;

            explicit
                EventQueue(const std::string& name, std::size_t maxQueueSize=SEDA_MAX_QUEUE_SIZE)
                : SEDA_INIT_LOGGER("seda.queue."+name),
                _name(name),
                _maxQueueSize(maxQueueSize) {}
            virtual ~EventQueue() {}

            const std::string& name() { return _name; }

            virtual IEvent::Ptr pop() throw(QueueEmpty) {
                boost::unique_lock<boost::mutex> lock(_mtx);

                while (empty()) {
                    _notEmptyCond.wait(lock);
                }
                assert(! empty());

                IEvent::Ptr e = _list.front(); _list.pop_front();
                if (empty()) {
                    _emptyCond.notify_one();
                } else {
                    _notEmptyCond.notify_one();
                }
                return e;
            }

            virtual IEvent::Ptr pop(unsigned long millis) throw(QueueEmpty) {
                boost::unique_lock<boost::mutex> lock(_mtx);

                while (empty()) {
                    boost::system_time const timeout=boost::get_system_time() + boost::posix_time::milliseconds(millis);
                    if (! _notEmptyCond.timed_wait(lock, timeout)) {
                        _emptyCond.notify_one();
                        throw QueueEmpty();
                    }
                }
                assert(! empty());

                IEvent::Ptr e = _list.front(); _list.pop_front();
                if (empty()) {
                    _emptyCond.notify_one();
                } else {
                    _notEmptyCond.notify_one();
                }
                return e;
            }

            virtual void push(const IEvent::Ptr& e) throw(QueueFull) {
                boost::unique_lock<boost::mutex> lock(_mtx);
               
                if (size() >= _maxQueueSize) {
                    throw QueueFull();
                } else {
                    _list.push_back(e);
                    _notEmptyCond.notify_one();
                }
            }

            virtual bool waitUntilEmpty() {
                boost::unique_lock<boost::mutex> lock(_mtx);
                while (! empty()) {
                    _emptyCond.wait(lock);
                }
                return true;
            }

            virtual bool waitUntilEmpty(unsigned long millis) {
                boost::unique_lock<boost::mutex> lock(_mtx);

                while (! empty()) {
                    boost::system_time const timeout=boost::get_system_time() + boost::posix_time::milliseconds(millis);
                    if (!_emptyCond.timed_wait(lock, timeout)) {
                        return false;
                    }
                }
                return true;
            }

            virtual bool waitUntilNotEmpty() {
                boost::unique_lock<boost::mutex> lock(_mtx);
                while (empty()) {
                    _notEmptyCond.wait(lock);
                }
                return true;
            }

            virtual bool waitUntilNotEmpty(unsigned long millis) {
                boost::unique_lock<boost::mutex> lock(_mtx);

                while (empty()) {
                    boost::system_time const timeout=boost::get_system_time() + boost::posix_time::milliseconds(millis);
                    if (!_notEmptyCond.timed_wait(lock, timeout)) {
                        return false;
                    }
                }
                return true;
            }

            /**
             * Removes all elements from the queue.
             * Warning: elements will be deleted!
             */
            virtual void clear() {
                boost::unique_lock<boost::mutex> lock(_mtx);
                while (!empty()) {
                    IEvent::Ptr e = _list.front();
                    _list.pop_front();
                }
                _emptyCond.notify_one();
            }

            virtual void wakeUpAll() {
                boost::unique_lock<boost::mutex> lock(_mtx);
                _notEmptyCond.notify_all();
                _emptyCond.notify_all();
            }

            virtual std::size_t size() const { return _list.size(); }
            bool empty() const { return _list.empty(); }

            std::size_t maxQueueSize() const { return _maxQueueSize; }
            void maxQueueSize(const std::size_t& max) { _maxQueueSize = max; }
        protected:
            SEDA_DECLARE_LOGGER();
            boost::mutex _mtx;
            boost::condition_variable _emptyCond;
            boost::condition_variable _notEmptyCond;

            std::list< IEvent::Ptr > _list;
        private:
            std::string _name;
            std::size_t _maxQueueSize;
    };
}

#endif
