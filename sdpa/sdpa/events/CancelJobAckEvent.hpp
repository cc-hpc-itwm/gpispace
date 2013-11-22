#ifndef SDPA_CANCELJOBACKEVENT_HPP
#define SDPA_CANCELJOBACKEVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>

#include <boost/serialization/base_object.hpp>

namespace sdpa
{
  namespace events
  {
    class CancelJobAckEvent : public JobEvent
    {
    public:
      typedef sdpa::shared_ptr<CancelJobAckEvent> Ptr;

      CancelJobAckEvent()
        : JobEvent ("", "", "")
        , _result ("unknown_result")
      {}

      CancelJobAckEvent ( const address_t& a_from
                        , const address_t& a_to
                        , const sdpa::job_id_t& a_job_id
                        , std::string const& result = "unknown_result"
                        )
        :  sdpa::events::JobEvent (a_from, a_to, a_job_id)
        , _result (result)
      {}

      std::string str() const
      {
        return "CancelJobAckEvent(" + job_id ().str () + ")";
      }
      std::string const& result() const
      {
        return _result;
      }
      virtual void handleBy (EventHandler* handler)
      {
        handler->handleCancelJobAckEvent (this);
      }

    private:
      std::string _result;

      friend class boost::serialization::access;
      template <typename Archive>
      void serialize (Archive& ar, const unsigned int)
      {
        ar & boost::serialization::base_object<JobEvent> (*this);
        ar & _result;
      }
    };
  }
}

#endif
