#ifndef SDPA_WORKER_REGISTRATION_EVENT_HPP
#define SDPA_WORKER_REGISTRATION_EVENT_HPP 1

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/capability.hpp>

#include <boost/optional.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/optional.hpp>

namespace sdpa
{
  namespace events
  {
    class WorkerRegistrationEvent : public MgmtEvent
    {
    public:
      typedef sdpa::shared_ptr<WorkerRegistrationEvent> Ptr;
      typedef enum {PUSH, REQ } service_model_t;

      WorkerRegistrationEvent()
        : MgmtEvent()
      {}

      WorkerRegistrationEvent
        ( const address_t& a_from
        , const address_t& a_to
        , const boost::optional<unsigned int>& capacity = boost::none
        , const capabilities_set_t& cpbset = capabilities_set_t()
        , const unsigned int& agent_rank = 0
        , const sdpa::worker_id_t& agent_uuid = ""
        )
          : MgmtEvent (a_from, a_to)
          , capacity_ (capacity)
          , cpbset_ (cpbset)
          , rank_ (agent_rank)
          , agent_uuid_ (agent_uuid)
      {}

      std::string str() const
      {
        return "WorkerRegistrationEvent";
      }

      const boost::optional<unsigned int>& capacity() const
      {
        return capacity_;
      }
      const unsigned int& rank() const
      {
        return rank_;
      }
      const capabilities_set_t& capabilities() const
      {
        return cpbset_;
      }
      const sdpa::worker_id_t& agent_uuid() const
      {
        return agent_uuid_;
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleWorkerRegistrationEvent (this);
      }

    private:
      boost::optional<unsigned int> capacity_;
      capabilities_set_t cpbset_;
      unsigned int rank_;
      sdpa::worker_id_t agent_uuid_;

      friend class boost::serialization::access;
      template <typename Archive>
      void serialize (Archive& ar, const unsigned int)
      {
        ar & boost::serialization::base_object<MgmtEvent> (*this);
        ar & capacity_;
        ar & cpbset_;
        ar & rank_;
        ar & agent_uuid_;
      }
    };
  }
}

#endif
