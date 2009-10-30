#ifndef SDPA_EVENT_HPP
#define SDPA_EVENT_HPP 1

#include <string>

#include <seda/IEvent.hpp>
#include <sdpa/memory.hpp>
#include <sdpa/events/EventVisitor.hpp>

namespace sdpa { namespace events {
  class SDPAEvent : public seda::IEvent {
    public:
      typedef sdpa::shared_ptr<SDPAEvent> Ptr;

      typedef std::string address_t;

      virtual ~SDPAEvent() {}

      const address_t & from() const { return from_; }
      address_t & from() { return from_; }
      const address_t & to() const { return to_; }
      address_t & to() { return to_; }

      virtual std::string str() const = 0;

      virtual void accept(EventVisitor *) = 0;
    protected:
      SDPAEvent()
        : IEvent()
        , from_()
        , to_()
      { }

      SDPAEvent(const SDPAEvent &other);
      SDPAEvent(const address_t &from, const address_t &to);
    private:
      address_t from_;
      address_t to_;
  };
}}

#endif // SDPA_EVENT_HPP
