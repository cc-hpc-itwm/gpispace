#ifndef GL_INTERNALEVENT_HPP
#define GL_INTERNALEVENT_HPP 1

#include <seda/IEvent.hpp>

namespace sdpa {
  namespace events {
    class InternalEvent : public seda::IEvent {
      public:
        typedef std::tr1::shared_ptr<InternalEvent> Ptr;
        InternalEvent(const std::string& conversation_id) { _conversation_id = conversation_id; }
        ~InternalEvent() {};

        const std::string& conversation_id() const { return _conversation_id; }
        std::string str() const { return std::string("Event ") + _conversation_id; }

      protected:
        std::string _conversation_id;
    };
  }
}

#endif // SDPA_INTERNALEVENT_HPP
