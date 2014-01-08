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
        ( const address_t& a_from
        , const address_t& a_to
        , const sdpa::job_id_t& a_job_id
        , const job_desc_t& a_description
        , const boost::optional<sdpa::job_id_t>& a_parent_id
        , const sdpa::worker_id_list_t& worker_list = sdpa::worker_id_list_t()
        )
          : SDPAEvent( a_from, a_to)
          , _job_id (a_job_id)
          , desc_ (a_description)
          , parent_ (a_parent_id)
          , worker_list_ (worker_list)
      {}

      std::string str() const
      {
        return "SubmitJobEvent(" + job_id ().str () + ")";
      }

      const sdpa::job_id_t& job_id() const
      {
        return _job_id;
      }
      const sdpa::job_desc_t& description() const
      {
        return desc_;
      }
      const boost::optional<sdpa::job_id_t>& parent_id() const
      {
        return parent_;
      }
      const sdpa::worker_id_list_t& worker_list() const
      {
        return worker_list_;
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleSubmitJobEvent (this);
      }

    private:
      sdpa::job_id_t _job_id;
      sdpa::job_desc_t desc_;
      boost::optional<sdpa::job_id_t> parent_;
      sdpa::worker_id_list_t worker_list_;
    };

    SAVE_CONSTRUCT_DATA_DEF (SubmitJobEvent, e)
    {
      SAVE_SDPAEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->job_id());
      SAVE_TO_ARCHIVE (e->description());
      SAVE_TO_ARCHIVE (e->parent_id());
      SAVE_TO_ARCHIVE (e->worker_list());
    }

    LOAD_CONSTRUCT_DATA_DEF (SubmitJobEvent, e)
    {
      LOAD_SDPAEVENT_CONSTRUCT_DATA (from, to);
      LOAD_FROM_ARCHIVE (sdpa::job_id_t, job_id);
      LOAD_FROM_ARCHIVE (sdpa::job_desc_t, description);
      LOAD_FROM_ARCHIVE (boost::optional<sdpa::job_id_t>, parent_id);
      LOAD_FROM_ARCHIVE (sdpa::worker_id_list_t, worker_list);

      ::new (e) SubmitJobEvent
          (from, to, job_id, description, parent_id, worker_list);
    }
  }
}

#endif
