#ifndef FHG_PLUGIN_CORE_KERNEL_HPP
#define FHG_PLUGIN_CORE_KERNEL_HPP 1

#include <fhg/util/thread/event.hpp>
#include <fhg/util/thread/queue.hpp>

namespace fhg
{
  namespace core
  {
    class wait_until_stopped
    {
    public:
      void wait();
      void stop();
      std::function<void()> make_request_stop();

      fhg::util::thread::event<> _stop_requested;
      fhg::util::thread::event<> _stopped;
    };

    class kernel_t
    {
    public:
      kernel_t (std::function<void()> request_stop);

      std::function<void()> _stop;
      void stop() { _stop(); }
    };
  }
}

#endif
