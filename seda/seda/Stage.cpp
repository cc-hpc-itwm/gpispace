#include <seda/Stage.hpp>

namespace seda
{
  void Stage::receive_and_perform()
  {
    while (true)
    {
      _strategy (_queue.get());
    }
  }

  Stage::Stage (boost::function<void (const boost::shared_ptr<IEvent>&)> strategy)
    : _queue()
    , _strategy (strategy)
    , _event_handler_thread
      (new boost::thread (&Stage::receive_and_perform, this))
  {}

  Stage::~Stage()
  {
    stop();
  }


  void Stage::stop()
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
}
