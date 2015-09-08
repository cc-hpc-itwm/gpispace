#pragma once

#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/events/EventHandler.hpp>
#include <sdpa/types.hpp>

namespace sdpa
{
  namespace events
  {
    class SubmitJobEvent : public SDPAEvent
    {
    public:
      typedef boost::shared_ptr<SubmitJobEvent> Ptr;

      SubmitJobEvent
        ( const boost::optional<sdpa::job_id_t>& a_job_id
        , const job_desc_t& a_description
        , std::set<worker_id_t> const& workers = {}
        )
          : SDPAEvent()
          , _job_id (a_job_id)
          , desc_ (a_description)
          , _workers (workers)
      {}

      const boost::optional<sdpa::job_id_t>& job_id() const
      {
        return _job_id;
      }
      const sdpa::job_desc_t& description() const
      {
        return desc_;
      }
      std::set<worker_id_t> const& workers() const
      {
        return _workers;
      }

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleSubmitJobEvent (source, this);
      }

    private:
      boost::optional<sdpa::job_id_t> _job_id;
      sdpa::job_desc_t desc_;
      std::set<worker_id_t> _workers;
    };

    SAVE_CONSTRUCT_DATA_DEF (SubmitJobEvent, e)
    {
      SAVE_SDPAEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->job_id());
      SAVE_TO_ARCHIVE (e->description());
      SAVE_TO_ARCHIVE (e->workers());
    }

    LOAD_CONSTRUCT_DATA_DEF (SubmitJobEvent, e)
    {
      LOAD_SDPAEVENT_CONSTRUCT_DATA();
      LOAD_FROM_ARCHIVE (boost::optional<sdpa::job_id_t>, job_id);
      LOAD_FROM_ARCHIVE (sdpa::job_desc_t, description);
      LOAD_FROM_ARCHIVE (std::set<sdpa::worker_id_t>, workers);

      ::new (e) SubmitJobEvent (job_id, description, workers);
    }
  }
}
