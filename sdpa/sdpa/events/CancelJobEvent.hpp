#ifndef SDPA_CANCELJOBEVENT_HPP
#define SDPA_CANCELJOBEVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/JobEvent.hpp>

#include <boost/serialization/base_object.hpp>

namespace sdpa
{
  namespace events
  {
    class CancelJobEvent : public JobEvent
    {
    public:
      typedef sdpa::shared_ptr<CancelJobEvent> Ptr;

      CancelJobEvent()
        : JobEvent ("", "", "")
        , _reason ("unknown")
      {}

      CancelJobEvent ( const address_t& a_from
                     , const address_t& a_to
                     , const sdpa::job_id_t& a_job_id
                     , const std::string& a_reason
                     )
        : sdpa::events::JobEvent (a_from, a_to, a_job_id)
        , _reason (a_reason)
      {}

      std::string str() const
      {
        return "CancelJobEvent(" + job_id ().str () + ")";
      }
      std::string const& reason() const
      {
        return _reason;
      }
      int priority() const
      {
        return 1;
      }
      virtual void handleBy (EventHandler* handler)
      {
        handler->handleCancelJobEvent (this);
      }

    private:
      std::string _reason;

      friend class boost::serialization::access;
      template <typename Archive>
      void serialize (Archive& ar, const unsigned int)
      {
        ar & boost::serialization::base_object<JobEvent> (*this);
        ar & _reason;
      }
    };
  }
}

#endif
