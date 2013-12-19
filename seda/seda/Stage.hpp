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
    Stage (boost::function<void (const boost::shared_ptr<IEvent>&)> strategy)
      : _queue()
      , _strategy (strategy)
      , _event_handler_thread
        (new boost::thread (&Stage::receive_and_perform, this))
    {}

    ~Stage()
    {
      stop();
    }

    void stop()
    {
      if (_event_handler_thread)
      {
        _event_handler_thread->interrupt();
        if (_event_handler_thread->joinable())
        {
          _event_handler_thread->join();
        }
        delete _event_handler_thread;
        _event_handler_thread = NULL;
      }
    }

    void send(const boost::shared_ptr<IEvent>& e)
    {
      _queue.put (e);
    }

  private:
    fhg::thread::queue<boost::shared_ptr<IEvent> > _queue;

    boost::function<void (const boost::shared_ptr<IEvent>&)> _strategy;

    void receive_and_perform()
    {
      while (true)
      {
        _strategy (_queue.get());
      }
    }
    boost::thread* _event_handler_thread;
  };
}

#endif
