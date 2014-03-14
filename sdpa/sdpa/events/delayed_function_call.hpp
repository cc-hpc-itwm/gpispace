// bernd.loerwald@itwm.fraunhofer.de

#ifndef SDPA_EVENTS_DELAYED_FUNCTION_CALL_HPP
#define SDPA_EVENTS_DELAYED_FUNCTION_CALL_HPP

#include <sdpa/events/SDPAEvent.hpp>

#include <boost/function.hpp>

namespace sdpa
{
  namespace events
  {
    class delayed_function_call : public sdpa::events::SDPAEvent
    {
    public:
      delayed_function_call (boost::function<void()> function)
        : SDPAEvent ("<<internal>>", "<<internal>>")
        , _function (function)
      {}

      virtual void handleBy (EventHandler*)
      {
        _function();
      }

    private:
      boost::function<void()> _function;
    };

    //! \note No serialization: Shall only be used within daemon, not over net
  }
}

#endif
