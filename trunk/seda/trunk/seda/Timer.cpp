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

