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
#include <boost/function.hpp>

#include <seda/IEvent.hpp>
#include <seda/Strategy.hpp>

#include <fhg/util/thread/queue.hpp>

namespace seda {
    class Stage {
    public:
      Stage (Strategy* strategy);
      Stage (boost::function<void (const IEvent::Ptr&)> strategy);

        virtual ~Stage();

        virtual void stop();

        virtual void send(const IEvent::Ptr& e) {
            _queue.put (e);
        }

    private:
      mutable boost::mutex _stop_mutex;

      fhg::thread::queue<IEvent::Ptr> _queue;

      boost::function<void (const IEvent::Ptr&)> _strategy;

      void receive_and_perform();
      boost::thread* _event_handler_thread;
    };
}

#endif // !SEDA_STAGE_HPP
