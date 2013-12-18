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

#ifndef SEDA_STAGE_HPP
#define SEDA_STAGE_HPP 1

#include <string>
#include <list>

#include <boost/thread.hpp>

#include <seda/shared_ptr.hpp>
#include <seda/SedaException.hpp>
#include <seda/IEvent.hpp>
#include <seda/EventQueue.hpp>
#include <seda/Strategy.hpp>

namespace seda {
    struct ThreadInfo;
    class Stage {
    private:
      typedef std::list<boost::thread*> ThreadPool;
    public:
        typedef seda::shared_ptr<Stage> Ptr;

        Stage(const std::string& name, Strategy::Ptr strategy, std::size_t maxPoolSize=1);

        virtual ~Stage();

        virtual void start();
        virtual void stop();

        virtual const std::string& name() const { return _name; }

        virtual Strategy::Ptr strategy() { return _strategy; }
        virtual const Strategy::Ptr strategy() const { return _strategy; }

        virtual void send(const IEvent::Ptr& e) {
            _queue->push(e);
        }
        virtual IEvent::Ptr recv() {
            return _queue->pop();
        }

    private:
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;

        SEDA_DECLARE_LOGGER();
        EventQueue::Ptr _queue;
        Strategy::Ptr _strategy;
        std::string _name;
        std::size_t _maxPoolSize;
        ThreadPool _threadPool;
        mutable mutex_type m_mutex;
    };
}

#endif // !SEDA_STAGE_HPP
