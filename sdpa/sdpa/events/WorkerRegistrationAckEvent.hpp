#ifndef SDPA_WORKER_REGISTRATION_ACK_EVENT_HPP
#define SDPA_WORKER_REGISTRATION_ACK_EVENT_HPP 1

#include <sdpa/events/MgmtEvent.hpp>

#include <boost/serialization/base_object.hpp>

namespace sdpa
{
  namespace events
  {
    class WorkerRegistrationAckEvent : public MgmtEvent
    {
    public:
      typedef sdpa::shared_ptr<WorkerRegistrationAckEvent> Ptr;

      WorkerRegistrationAckEvent()
        : MgmtEvent()
      {}
      WorkerRegistrationAckEvent ( const address_t& a_from
                                 , const address_t& a_to
                                 )
        : MgmtEvent (a_from, a_to)
      {}

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleWorkerRegistrationAckEvent (this);
      }

      std::string str() const
      {
        return "WorkerRegistrationAckEvent";
      }

    private:
      friend class boost::serialization::access;
      template <typename Archive>
      void serialize (Archive& ar, const unsigned int)
      {
        ar & boost::serialization::base_object<MgmtEvent> (*this);
      }
    };
  }
}

#endif
