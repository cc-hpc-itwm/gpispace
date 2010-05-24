/*
 * =====================================================================================
 *
 *       Filename:  SDPAEvent.hpp
 *
 *    Description:  SDPAEvent
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */
#ifndef SDPA_EVENT_HPP
#define SDPA_EVENT_HPP 1

#include <string>

#include <seda/IEvent.hpp>
#include <sdpa/memory.hpp>
#include <sdpa/events/EventHandler.hpp>

namespace sdpa { namespace events {
  class SDPAEvent : public seda::IEvent {
    public:
      typedef sdpa::shared_ptr<SDPAEvent> Ptr;

      typedef std::string address_t;
	  typedef std::string message_id_type;

      virtual ~SDPAEvent() {}

      const address_t & from() const { return from_; }
      address_t & from() { return from_; }
      const address_t & to() const { return to_; }
      address_t & to() { return to_; }
	  const message_id_type &id() const { return id_; }
	  message_id_type &id() { return id_; }

      virtual std::string str() const = 0;

      virtual void handleBy(EventHandler *) = 0;
    protected:
      SDPAEvent()
        : IEvent()
        , from_()
        , to_()
		, id_()
      { }

      SDPAEvent(const SDPAEvent &other);
      SDPAEvent(const address_t &from, const address_t &to);
      SDPAEvent(const address_t &from, const address_t &to, const message_id_type &mid);
    private:
      address_t from_;
      address_t to_;
	  message_id_type id_;
  };
}}

#endif // SDPA_EVENT_HPP
