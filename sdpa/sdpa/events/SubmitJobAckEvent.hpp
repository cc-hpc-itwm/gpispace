#ifndef SDPA_SubmitJobAckEvent_HPP
#define SDPA_SubmitJobAckEvent_HPP

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>

namespace sdpa
{
  namespace events
  {
    class SubmitJobAckEvent : public JobEvent
    {
    public:
      typedef sdpa::shared_ptr<SubmitJobAckEvent> Ptr;

      SubmitJobAckEvent()
        : JobEvent ("", "", "")
      {}

      SubmitJobAckEvent ( const address_t& a_from
                        , const address_t& a_to
                        , const sdpa::job_id_t & a_job_id
                        )
        : JobEvent (a_from, a_to, a_job_id)
      {}

      std::string str() const
      {
        return "SubmitJobAckEvent(" + job_id ().str () + ")";
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleSubmitJobAckEvent (this);
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
