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

#ifndef SEDA_EVENT_QUEUE_HPP
#define SEDA_EVENT_QUEUE_HPP 1

#include <list>
#include <assert.h>

#include <boost/thread.hpp>

#include <seda/common.hpp>
#include <seda/shared_ptr.hpp>
#include <seda/IEventQueue.hpp>
#include <seda/IEvent.hpp>

#define WAIT_INFINITE 0xFFFFffff

namespace seda {
    class EventQueue : public IEventQueue {
        public:
            typedef seda::shared_ptr< EventQueue > Ptr;

            explicit
                EventQueue(const std::string& a_name)
                : SEDA_INIT_LOGGER("seda.queue."+a_name),
                _name(a_name) {}

            const std::string& name() { return _name; }

            virtual IEvent::Ptr pop() {
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

            virtual void push(const IEvent::Ptr& e) {
                boost::unique_lock<boost::mutex> lock(_mtx);

                _list.push_back(e);
                _notEmptyCond.notify_one();
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

        protected:
            SEDA_DECLARE_LOGGER();
            boost::mutex _mtx;
            boost::condition_variable _emptyCond;
            boost::condition_variable _notEmptyCond;

            std::list< IEvent::Ptr > _list;
        private:
            std::string _name;
    };
}

#endif
