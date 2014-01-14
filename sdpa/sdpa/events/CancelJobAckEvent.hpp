#ifndef SDPA_CANCELJOBACKEVENT_HPP
#define SDPA_CANCELJOBACKEVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/Serialization.hpp>

namespace sdpa
{
  namespace events
  {
    class CancelJobAckEvent : public JobEvent
    {
    public:
      typedef boost::shared_ptr<CancelJobAckEvent> Ptr;

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
        return "CancelJobAckEvent(" + job_id () + ")";
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
    };

    SAVE_CONSTRUCT_DATA_DEF (CancelJobAckEvent, e)
    {
      SAVE_JOBEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->result());
    }

    LOAD_CONSTRUCT_DATA_DEF (CancelJobAckEvent, e)
    {
      LOAD_JOBEVENT_CONSTRUCT_DATA (from, to, job_id);
      LOAD_FROM_ARCHIVE (std::string, result);

      ::new (e) CancelJobAckEvent (from, to, job_id, result);
    }
  }
}

#endif
