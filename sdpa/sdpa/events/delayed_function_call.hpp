// bernd.loerwald@itwm.fraunhofer.de

#ifndef SDPA_EVENTS_DELAYED_FUNCTION_CALL_HPP
#define SDPA_EVENTS_DELAYED_FUNCTION_CALL_HPP

#include <sdpa/events/SDPAEvent.hpp>

#include <functional>

namespace sdpa
{
  namespace events
  {
    class delayed_function_call : public sdpa::events::SDPAEvent
    {
    public:
      delayed_function_call (std::function<void()> function)
        : SDPAEvent ("<<internal>>", "<<internal>>")
        , _function (function)
      {}

      virtual void handleBy (EventHandler*) override
      {
        _function();
      }

    private:
      std::function<void()> _function;
    };

    //! \note No serialization: Shall only be used within daemon, not over net
  }
}

#endif
