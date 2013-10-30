/* 
   Copyright (C) 2009 Alexander Petry <alexander.petry@itwm.fraunhofer.de>.

   This file is part of seda.

   seda is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   seda is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with seda; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  

*/

#ifndef SEDA_EVENT_COUNT_STRATEGY_HPP
#define SEDA_EVENT_COUNT_STRATEGY_HPP 1

#include <boost/thread.hpp>

#include <seda/StrategyDecorator.hpp>

namespace seda {
    class EventCountStrategy : public StrategyDecorator {
        public:
            typedef seda::shared_ptr<EventCountStrategy> Ptr;

            explicit
                EventCountStrategy(const Strategy::Ptr& s)
                : StrategyDecorator(s->name()+".count", s),
                _count(0) {}

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
