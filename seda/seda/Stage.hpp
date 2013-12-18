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
#include <seda/IEvent.hpp>
#include <seda/Strategy.hpp>

#include <fhg/util/thread/queue.hpp>

namespace seda {
    class Stage {
    public:
        typedef seda::shared_ptr<Stage> Ptr;

        Stage(const std::string& name, Strategy::Ptr strategy);

        virtual ~Stage();

        virtual void start();
        virtual void stop();

        virtual const std::string& name() const { return _name; }

        virtual void send(const IEvent::Ptr& e) {
            _queue.put (e);
        }

    private:
      void receive_and_perform();
      boost::thread* _event_handler_thread;

      mutable boost::mutex _start_stop_mutex;

      fhg::thread::queue<IEvent::Ptr> _queue;

        Strategy::Ptr _strategy;
        std::string _name;
    };
}

#endif // !SEDA_STAGE_HPP
