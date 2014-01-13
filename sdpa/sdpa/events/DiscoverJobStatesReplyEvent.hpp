#ifndef SDPA_DISCOVER_PENDING_ACT_REPLY_EVENT_HPP
#define SDPA_DISCOVER_PENDING_ACT_REPLY_EVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>
#include <we/type/value/show.hpp>
#include <we/type/value/read.hpp>
#include <sstream>

namespace sdpa
{
  namespace events
  {
    class DiscoverJobStatesReplyEvent : public sdpa::events::JobEvent
    {
    public:
      typedef boost::shared_ptr<JobEvent> Ptr;

      DiscoverJobStatesReplyEvent ( const address_t& a_from
                                    , const address_t& a_to
                                    , const sdpa::job_id_t& a_job_id
                                    , const std::string& discover_id
                                    , const pnet::type::value::value_type& discover_result
                                    )
        : sdpa::events::JobEvent (a_from, a_to, a_job_id)
        , discover_id_(discover_id)
        , discover_result_(discover_result)
      {}

      std::string str() const
      {
        return "DiscoverJobStatesReplyEvent(" + job_id ().str () + ")";
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleDiscoverJobStatesReplyEvent (this);
      }

      const std::string& discover_id() const { return discover_id_; }
      const pnet::type::value::value_type& discover_result() const
      {
        return discover_result_;
      }
    private:
      std::string discover_id_;
      pnet::type::value::value_type discover_result_;
    };

    SAVE_CONSTRUCT_DATA_DEF (DiscoverJobStatesReplyEvent, e)
     {
       SAVE_JOBEVENT_CONSTRUCT_DATA (e);
       SAVE_TO_ARCHIVE (e->discover_id());
       std::ostringstream oss;
       oss << pnet::type::value::show (e->discover_result());
       std::string res(oss.str());
       SAVE_TO_ARCHIVE (res);
     }

     LOAD_CONSTRUCT_DATA_DEF (DiscoverJobStatesReplyEvent, e)
     {
       LOAD_JOBEVENT_CONSTRUCT_DATA (from, to, job_id);
       LOAD_FROM_ARCHIVE (std::string, disc_id);
       LOAD_FROM_ARCHIVE (std::string, res);
       pnet::type::value::value_type disc_res(pnet::type::value::read (res));
       ::new (e) DiscoverJobStatesReplyEvent (from, to, job_id, disc_id, disc_res);
     }
  }
}

#endif
