#ifndef SEDA_STAGE_HPP
#define SEDA_STAGE_HPP

#include <seda/IEvent.hpp>

#include <fhg/util/thread/queue.hpp>

#include <boost/thread.hpp>
#include <boost/function.hpp>

namespace seda
{
  class Stage
  {
  public:
    Stage (boost::function<void (const IEvent::Ptr&)> strategy);

    virtual ~Stage();

    virtual void stop();

    virtual void send(const IEvent::Ptr& e) {
      _queue.put (e);
    }

  private:
    fhg::thread::queue<IEvent::Ptr> _queue;

    boost::function<void (const IEvent::Ptr&)> _strategy;

    void receive_and_perform();
    boost::thread* _event_handler_thread;
  };
}

#endif
