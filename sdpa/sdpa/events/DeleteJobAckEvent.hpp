#ifndef SDPA_DeleteJobAckEvent_HPP
#define SDPA_DeleteJobAckEvent_HPP 1

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/JobEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class DeleteJobAckEvent : public JobEvent
    {
    public:
      typedef sdpa::shared_ptr<DeleteJobAckEvent> Ptr;

      DeleteJobAckEvent()
        : JobEvent ("", "", "")
      {}

      DeleteJobAckEvent ( const address_t& a_from
                        , const address_t& a_to
                        , const sdpa::job_id_t& a_job_id
                        )
        :  sdpa::events::JobEvent( a_from, a_to, a_job_id)
      {}

      std::string str() const
      {
        return "DeleteJobAckEvent(" + job_id().str () + ")";
      }

      virtual void handleBy(EventHandler* handler)
      {
        handler->handleDeleteJobAckEvent (this);
      }

    private:
      friend class boost::serialization::access;
      template <typename Archive>
      void serialize (Archive& ar, const unsigned int)
      {
        ar & boost::serialization::base_object<JobEvent> (*this);
      }
    };
  }
}

#endif
