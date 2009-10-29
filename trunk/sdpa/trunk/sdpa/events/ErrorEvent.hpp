#ifndef SDPA_ERROREVENT_HPP
#define SDPA_ERROREVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/events/EventVisitor.hpp>

namespace sdpa { namespace events {
// this could be replaced by a small macro
#ifdef USE_BOOST_SC
  class ErrorEvent : public MgmtEvent, public sc::event<ErrorEvent>
#else
  class ErrorEvent : public MgmtEvent
#endif
  {
    public:
      typedef sdpa::shared_ptr<ErrorEvent> Ptr;

      enum error_code_t {
          SDPA_ENOERROR = 0,
          SDPA_ENOJOBAVAIL,
          SDPA_EWORKERNOTREG,
          SDPA_EBUSY,
          SDPA_EAGAIN,
          SDPA_EUNKNOWN
      };

      ErrorEvent()
        : MgmtEvent()
        , error_code_(SDPA_EUNKNOWN)
        , reason_("unknown reason")
      {}

      ErrorEvent(const address_t &a_from
               , const address_t &a_to
               , const error_code_t &a_error_code
               , const std::string& a_reason = "unknown reason")
        : MgmtEvent(a_from, a_to)
        , error_code_(a_error_code)
        , reason_(a_reason)
      {}

      ~ErrorEvent() {}

      const std::string &reason() const { return reason_; }
      std::string &reason() { return reason_; }
      const error_code_t &error_code() const { return error_code_; }
      error_code_t &error_code() { return error_code_; }

      std::string str() const { return "ErrorEvent"; }

      virtual void accept(EventVisitor *visitor)
      {
        visitor->visitErrorEvent(this);
      }
    private:
      error_code_t error_code_;
      std::string reason_;
  };
}}
#endif
