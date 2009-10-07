#ifndef SEDA_EVENT_COUNT_STRATEGY_HPP
#define SEDA_EVENT_COUNT_STRATEGY_HPP 1

#include <boost/thread.hpp>

#include <seda/StrategyDecorator.hpp>

namespace seda {
    class EventCountStrategy : public StrategyDecorator {
        public:
            typedef std::tr1::shared_ptr<EventCountStrategy> Ptr;

            explicit
                EventCountStrategy(const Strategy::Ptr& s)
                : StrategyDecorator(s->name()+".count", s),
                _count(0) {}

            ~EventCountStrategy() {}

            void perform(const IEvent::Ptr& e) {
                inc();
                SEDA_LOG_DEBUG("event counted");
                StrategyDecorator::perform(e);
            }

            std::size_t count() const { return _count; }
            void reset() {
                boost::unique_lock<boost::mutex> lock(_mtx);
                _count=0;
            }
            void inc() {
                boost::unique_lock<boost::mutex> lock(_mtx);
                _count++;
                _cond.notify_all();
            }
            bool wait(std::size_t targetValue) {
                boost::unique_lock<boost::mutex> lock(_mtx);
                while (count() < targetValue) {
                    _cond.wait(lock);
                }
                return true;
            }
            bool wait(std::size_t targetValue, unsigned long millis) {
                boost::unique_lock<boost::mutex> lock(_mtx);

                while (count() < targetValue) {
                    boost::system_time const timeout=boost::get_system_time() + boost::posix_time::milliseconds(millis);
                    if (!_cond.timed_wait(lock, timeout)) {
                        return false;
                    }
                }
                return true;
            }

            bool waitNoneZero() { return wait(1); }
            bool waitNoneZero(unsigned long millis) { return wait(1, millis); }
        private:
            std::size_t _count;
            boost::mutex _mtx;
            boost::condition_variable _cond;
    };
}

#endif // !SEDA_EVENT_COUNT_STRATEGY_HPP
