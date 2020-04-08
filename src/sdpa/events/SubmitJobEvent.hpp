#pragma once

#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/events/EventHandler.hpp>
#include <sdpa/types.hpp>

#include <we/type/activity.hpp>
#include <we/type/net.hpp>

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
        , we::type::activity_t activity
        , boost::optional<std::string> const& implementation
        , std::set<worker_id_t> const& workers = {}
        )
          : SDPAEvent()
          , _job_id (a_job_id)
          , _activity (std::move (activity))
          , _implementation (implementation)
          , _workers (workers)
      {}

      const boost::optional<sdpa::job_id_t>& job_id() const
      {
        return _job_id;
      }
      const we::type::activity_t& activity() const
      {
        return _activity;
      }
      const boost::optional<std::string>& implementation() const
      {
        return _implementation;
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
      we::type::activity_t _activity;
      boost::optional<std::string> _implementation;
      std::set<worker_id_t> _workers;
    };

    SAVE_CONSTRUCT_DATA_DEF (SubmitJobEvent, e)
    {
      SAVE_SDPAEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->job_id());
      SAVE_TO_ARCHIVE (e->activity());
      SAVE_TO_ARCHIVE (e->implementation());
      SAVE_TO_ARCHIVE (e->workers());
    }

    LOAD_CONSTRUCT_DATA_DEF (SubmitJobEvent, e)
    {
      LOAD_SDPAEVENT_CONSTRUCT_DATA();
      LOAD_FROM_ARCHIVE (boost::optional<sdpa::job_id_t>, job_id);
      LOAD_FROM_ARCHIVE (we::type::activity_t, activity);
      LOAD_FROM_ARCHIVE (boost::optional<std::string>, implementation);
      LOAD_FROM_ARCHIVE (std::set<sdpa::worker_id_t>, workers);

      ::new (e) SubmitJobEvent (job_id, std::move (activity), implementation, workers);
    }
  }
}
