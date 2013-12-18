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
        : SEDA_INIT_LOGGER("seda.stage."+a_name)
        , _event_handler_thread (NULL)
        , _queue()
        , _strategy(a_strategy)
        , _name(a_name)
    {
    }

    Stage::~Stage() {
        try {
            // stop the running threads and delete them
            stop();
        } catch (const std::exception& e) {
            SEDA_LOG_ERROR("stopping failed: " << e.what());
        } catch (...) {
            SEDA_LOG_ERROR("stopping failed: unknown reason");
        }
    }

    void
    Stage::start() {
      lock_type lock (m_mutex);

      if (!_event_handler_thread)
      {
        _strategy->onStageStart (name());

        _event_handler_thread =
          new boost::thread (&Stage::receive_and_perform, this);
      } // else == noop
    }

    void
    Stage::stop() {
      lock_type lock (m_mutex);

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
