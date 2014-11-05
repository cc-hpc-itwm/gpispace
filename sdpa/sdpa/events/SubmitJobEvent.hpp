#ifndef SDPA_SubmitJobEvent_HPP
#define SDPA_SubmitJobEvent_HPP

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
        , const sdpa::worker_id_list_t& worker_list = sdpa::worker_id_list_t()
        )
          : SDPAEvent()
          , _job_id (a_job_id)
          , desc_ (a_description)
          , worker_list_ (worker_list)
      {}

      const boost::optional<sdpa::job_id_t>& job_id() const
      {
        return _job_id;
      }
      const sdpa::job_desc_t& description() const
      {
        return desc_;
      }
      const sdpa::worker_id_list_t& worker_list() const
      {
        return worker_list_;
      }

      virtual void handleBy
        (std::string const& source, EventHandler* handler) override
      {
        handler->handleSubmitJobEvent (source, this);
      }

    private:
      boost::optional<sdpa::job_id_t> _job_id;
      sdpa::job_desc_t desc_;
      sdpa::worker_id_list_t worker_list_;
    };

    SAVE_CONSTRUCT_DATA_DEF (SubmitJobEvent, e)
    {
      SAVE_SDPAEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->job_id());
      SAVE_TO_ARCHIVE (e->description());
      SAVE_TO_ARCHIVE (e->worker_list());
    }

    LOAD_CONSTRUCT_DATA_DEF (SubmitJobEvent, e)
    {
      LOAD_SDPAEVENT_CONSTRUCT_DATA();
      LOAD_FROM_ARCHIVE (boost::optional<sdpa::job_id_t>, job_id);
      LOAD_FROM_ARCHIVE (sdpa::job_desc_t, description);
      LOAD_FROM_ARCHIVE (sdpa::worker_id_list_t, worker_list);

      ::new (e) SubmitJobEvent (job_id, description, worker_list);
    }
  }
}

#endif
