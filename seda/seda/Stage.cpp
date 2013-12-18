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

#include <csignal>
#include <pthread.h>


namespace seda {
  namespace
  {
    class StageWorker {
    public:
        StageWorker(const std::string& id, Stage* s) :
            SEDA_INIT_LOGGER(id),
            _stage(s),
            _busy(false),
            _stopped(false)
        { }

        void stop() { _stopped = true; }
        void operator()() { run(); }
        void run();
        bool busy() const { return _busy; }

    private:
        SEDA_DECLARE_LOGGER();
        bool stopped() { return _stopped; }

        Stage* _stage;
        bool _busy;
        bool _stopped;
    };

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
                IEvent::Ptr e = _stage->recv();
                _busy = true;

                _stage->strategy()->perform(e);

                _busy = false;
            } catch (const boost::thread_interrupted &irq) {
                break;
            } catch (const std::exception& ex) {
                SEDA_LOG_ERROR("strategy execution failed: " << ex.what());
            } catch (...) {
                SEDA_LOG_ERROR("strategy execution failed: unknown reason");
            }
        }
    }
  }

  struct ThreadInfo
  {
    boost::thread *thread;
    StageWorker  *worker;
  };

    Stage::Stage(const std::string& a_name, Strategy::Ptr a_strategy, std::size_t a_maxPoolSize)
        : SEDA_INIT_LOGGER("seda.stage."+a_name),
          _queue(new EventQueue),
          _strategy(a_strategy),
          _name(a_name),
          _maxPoolSize(a_maxPoolSize)
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

        try {
            /* log input queue if not empty */
            if (!_queue->empty()) {
                DMLOG (TRACE, "cleaning up input queue");
                while (!_queue->empty()) {
                    IEvent::Ptr e(_queue->pop());
                    DMLOG (TRACE, "removed incoming event: " << e->str());
                }
                DMLOG (TRACE, "done");
            }
        } catch (...) {}
    }

    void
    Stage::start() {
      lock_type lock (m_mutex);
        if (_threadPool.empty()) {
            _strategy->onStageStart(name());

            // initialize and start worker threads
            for (std::size_t tId = 0; tId < _maxPoolSize; ++tId) {
                std::ostringstream sstr;
                sstr << "seda.stage." << name() << ".worker." << tId;
                ThreadInfo *i = new ThreadInfo;
                i->worker = new StageWorker(sstr.str(), this);
                i->thread = new boost::thread(boost::ref(*i->worker));
                _threadPool.push_back(i);
            }
        } // else == noop
    }

    void
    Stage::stop() {
      lock_type lock (m_mutex);
        if (_threadPool.empty()) {
            return;
        }

        for (ThreadPool::iterator it(_threadPool.begin()); it != _threadPool.end(); ++it) {
            (*it)->worker->stop(); // signal threads to stop
            (*it)->thread->interrupt();
        }

        while (!_threadPool.empty()) {
            ThreadInfo *i(_threadPool.front()); _threadPool.pop_front();

            i->thread->join();
            delete i->thread;
            delete i->worker;
            delete i;
        }
        _strategy->onStageStop(name());
    }

    void
    Stage::send(const std::string& stageName, const seda::IEvent::Ptr& e) {
        StageRegistry::instance().lookup(stageName)->send(e);
    }
}
