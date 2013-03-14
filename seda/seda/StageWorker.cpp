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

#include "StageWorker.hpp"
#include "Stage.hpp"
#include "IEvent.hpp"
#include "EventNotSupported.hpp"
#include "SystemEvent.hpp"
#include "StageRegistry.hpp"

#include <csignal>
#include <pthread.h>

namespace seda {
    void StageWorker::run() {
      sigset_t signals_to_block;
      sigaddset (&signals_to_block, SIGTERM);
      sigaddset (&signals_to_block, SIGSEGV);
      sigaddset (&signals_to_block, SIGHUP);
      sigaddset (&signals_to_block, SIGPIPE);
      sigaddset (&signals_to_block, SIGINT);
      pthread_sigmask (SIG_BLOCK, &signals_to_block, NULL);

        while (!stopped()) {
            try {
                IEvent::Ptr e = _stage->recv(_stage->timeout());
                _busy = true;

                try {
                  //DLOG(TRACE, "handling event in stage " << _stage->name());
                  //                  SEDA_LOG_DEBUG("handling event in stage " << _stage->name());

                    _stage->strategy()->perform(e);

                  //DLOG(TRACE, "handled event in stage " << _stage->name());
                  //SEDA_LOG_DEBUG("handled event in stage " << _stage->name());
                } catch (const seda::EventNotSupported&) {
                    Stage::Ptr systemEventHandler(StageRegistry::instance().lookup(_stage->getErrorHandler()));
                    if (systemEventHandler.get() != _stage) {
                        systemEventHandler->send(e);
                    } else {
                        SEDA_LOG_FATAL("received a SystemEvent, but it could not be handled!");
                    }
                }

                _busy = false;
            } catch (const seda::QueueEmpty&) {
                // ignore
            } catch (const seda::QueueFull&) {
                SEDA_LOG_ERROR("event discarded due to overflow protection");
            } catch (const seda::StageNotFound& snf) {
                SEDA_LOG_ERROR("event not handled, stage `" << snf.stageName() << "' could not be found!");
            } catch (const boost::thread_interrupted &irq) {
                break;
            } catch (const std::exception& ex) {
                SEDA_LOG_ERROR("strategy execution failed: " << ex.what());
            } catch (...) {
                SEDA_LOG_ERROR("strategy execution failed: unknown reason");
            }
        }
        SEDA_LOG_DEBUG("terminating");
    }
}
