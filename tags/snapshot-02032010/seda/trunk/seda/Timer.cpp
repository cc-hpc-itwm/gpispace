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

#include "Timer.hpp"
#include "TimerEvent.hpp"
#include "StageRegistry.hpp"

using namespace seda;

void
Timer::start() {
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);

    if (_active)
        return;
    _thread = boost::thread(boost::ref(*this));
}

void
Timer::stop() {
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    _thread.interrupt();     
    while (_thread.get_id() != boost::thread::id()) {
        _cond.wait(lock);
    }
    _active = false;
}

bool
Timer::active() {
    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    return _active;
}

void
Timer::operator()() {
    {
        boost::unique_lock<boost::recursive_mutex> lock(_mtx);
        if (_active) {
            return;
        } else {
            _active = true;
        }
    }

    while (active()) {
        try {
            boost::this_thread::sleep(boost::get_system_time() + interval());

            try {
                seda::Stage::Ptr stage(StageRegistry::instance().lookup(targetStage()));
                stage->send(seda::TimerEvent::Ptr(new seda::TimerEvent(tag())));
            } catch(const seda::StageNotFound& ) {
                std::clog << "stage `" << targetStage() << "' could not be found!" << std::endl;
                break;
            } catch(...) {
               // ignore any failures during send
            }
        } catch (const boost::thread_interrupted& ) {
            break;
        }
    }

    boost::unique_lock<boost::recursive_mutex> lock(_mtx);
    _thread.detach();
    _cond.notify_one();
    _active = false;
}

