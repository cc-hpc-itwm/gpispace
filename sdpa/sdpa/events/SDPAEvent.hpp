#ifndef SDPA_EVENT_HPP
#define SDPA_EVENT_HPP 1

#include <string>

#include <sdpa/logging.hpp>
#include <seda/IEvent.hpp>
#include <sdpa/memory.hpp>
#include <sdpa/events/EventHandler.hpp>

#include <boost/system/error_code.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/assume_abstract.hpp>

namespace sdpa
{
  namespace events
  {
    class SDPAEvent : public seda::IEvent
    {
    public:
      typedef sdpa::shared_ptr<SDPAEvent> Ptr;

      typedef std::string address_t;

      const address_t& from() const
      {
        return from_;
      }
      const address_t& to() const
      {
        return to_;
      }
      //! \todo eliminate, used in Orchestrator::notifySubscriber
      address_t& to()
      {
        return to_;
      }

      virtual std::string str() const = 0;
      virtual void handleBy (EventHandler*) = 0;

    protected:
      SDPAEvent (const address_t & a_from, const address_t &a_to)
        : from_ (a_from)
        , to_ (a_to)
      {}

    private:
      address_t from_;
      address_t to_;

      friend class boost::serialization::access;
      template <class Archive>
      void serialize (Archive & ar, unsigned int)
      {
      }
    };
  }
}

#include <sdpa/events/Serialization.hpp>

#endif
