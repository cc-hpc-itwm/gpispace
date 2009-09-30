#ifndef SDPA_ERROREVENT_HPP
#define SDPA_ERROREVENT_HPP 1

// this should be added to CMakeLists.txt and put into the global configuration header
#define USE_BOOST_SC 1

#if USE_BOOST_SC == 1
#include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif

#include <sdpa/events/MgmtEvent.hpp>

namespace sdpa { namespace events {
// this could be replaced by a small macro
#if USE_BOOST_SC == 1
  class ErrorEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::ErrorEvent>
#else
  class ErrorEvent : public sdpa::events::MgmtEvent
#endif
  {
    public:
      typedef sdpa::shared_ptr<ErrorEvent> Ptr;

      enum error_code_t {
          SDPA_ENOERROR = 0,
          SDPA_ENOJOBAVAIL,
          SDPA_EWORKERNOTREG,
          SDPA_EBUSY,
          SDPA_EAGAIN
      };

      ErrorEvent(const address_t &from, const address_t &to, const error_code_t &error_code, const std::string& reason = "unknown reason")
      : MgmtEvent(from, to), error_code_(error_code), reason_(reason)
      {
      }

      ~ErrorEvent()
      {
      }

      const std::string& reason() const { return reason_; }
      const error_code_t error_code() const { return error_code_; }

      std::string str() const { return "ErrorEvent"; }
    private:
      error_code_t error_code_;
      std::string reason_;
  };
}}
#endif
