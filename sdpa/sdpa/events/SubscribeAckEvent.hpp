#ifndef SDPA_SUBSCRIBE_ACK_EVENT_HPP
#define SDPA_SUBSCRIBE_ACK_EVENT_HPP 1

#include <sdpa/events/MgmtEvent.hpp>

#include <boost/serialization/base_object.hpp>

namespace sdpa
{
  namespace events
  {
    class SubscribeAckEvent : public MgmtEvent
    {
    public:
      typedef sdpa::shared_ptr<SubscribeAckEvent> Ptr;

      SubscribeAckEvent()
        : MgmtEvent()
      {}

      SubscribeAckEvent ( const address_t& a_from
                        , const address_t& a_to
                        , const job_id_list_t& listJobIds
                        )
        : MgmtEvent (a_from, a_to)
        , listJobIds_ (listJobIds)
      { }

      const sdpa::job_id_list_t& listJobIds() const
      {
        return listJobIds_;
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleSubscribeAckEvent (this);
      }

      std::string str() const
      {
        return "SubscribeAckEvent";
      }

    private:
      sdpa::job_id_list_t listJobIds_;

      friend class boost::serialization::access;
      template <typename Archive>
      void serialize (Archive& ar, const unsigned int)
      {
        ar & boost::serialization::base_object<MgmtEvent> (*this);
        ar & listJobIds_;
      }
    };
  }
}

#endif
