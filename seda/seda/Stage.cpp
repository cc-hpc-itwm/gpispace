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

#include "Stage.hpp"
#include "IEvent.hpp"
#include "EventQueue.hpp"
#include "StageRegistry.hpp"

#include <boost/foreach.hpp>

namespace seda {
  void Stage::receive_and_perform()
  {
    while (true)
    {
      _strategy->perform (_queue.pop());
    }
  }

    Stage::Stage(const std::string& a_name, Strategy::Ptr a_strategy)
        : _event_handler_thread (NULL)
        , _queue()
        , _strategy(a_strategy)
        , _name(a_name)
    {
    }

    Stage::~Stage() {
      stop();
    }

    void
    Stage::start() {
      boost::mutex::scoped_lock _ (_start_stop_mutex);

      if (!_event_handler_thread)
      {
        _strategy->onStageStart (name());

        _event_handler_thread =
          new boost::thread (&Stage::receive_and_perform, this);
      } // else == noop
    }

    void
    Stage::stop() {
      boost::mutex::scoped_lock _ (_start_stop_mutex);

      if (_event_handler_thread)
      {
        _event_handler_thread->interrupt();
        if (_event_handler_thread->joinable())
        {
          _event_handler_thread->join();
        }
        delete _event_handler_thread;
        _event_handler_thread = NULL;

        _strategy->onStageStop (name());
      }
    }
}
