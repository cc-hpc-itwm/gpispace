#include <plugin/core/kernel.hpp>

#include <thread>

namespace fhg
{
  namespace core
  {
    void wait_until_stopped::wait()
    {
      //! \note this strange looking thread is required to avoid a
      //! deadlock that may happen when we are blocked within this
      //! function and handle a terminating signal as well (which in
      //! turn calls stop()).
      //!
      //! the deadlock is between the events '_stop_requested' and '_stopped',
      //! we first block in '_stop_requested.wait()' and are released by
      //! '_stop_requested.notify()', however after that call we
      //! block again in '_stopped.wait()' and get never released.
      std::thread ([this]
                  {
                    _stop_requested.wait();
                    _stopped.notify();
                  }
                  ).join();
    }

    void wait_until_stopped::stop()
    {
      std::thread ([this]
                  {
                    _stop_requested.notify();
                    _stopped.wait();
                  }
                  ).join();
    }

    std::function<void()> wait_until_stopped::make_request_stop()
    {
      return [this] { stop(); };
    }

    kernel_t::kernel_t (std::function<void()> request_stop)
      : _stop (request_stop)
    {
    }
  }
}
