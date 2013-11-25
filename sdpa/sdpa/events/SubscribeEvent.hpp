#ifndef SDPA_SUBSCRIBE_EVENT_HPP
#define SDPA_SUBSCRIBE_EVENT_HPP 1

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/types.hpp>

#include <boost/serialization/base_object.hpp>

namespace sdpa
{
  namespace events
  {
    class SubscribeEvent : public MgmtEvent
    {
    public:
      typedef sdpa::shared_ptr<SubscribeEvent> Ptr;

      SubscribeEvent()
        : MgmtEvent()
      {}

      SubscribeEvent ( const address_t& a_from
                     , const address_t& a_to
                     , const job_id_list_t& listJobIds
                     )
        : MgmtEvent (a_from, a_to)
        , listJobIds_ (listJobIds)
      {}

      std::string str() const
      {
        return "SubscribeEvent";
      }

      const sdpa::agent_id_t& subscriber() const
      {
        return from();
      }
      const sdpa::job_id_list_t& listJobIds() const
      {
        return listJobIds_;
      }

      virtual void handleBy (EventHandler* handler)
      {
    	handler->handleSubscribeEvent (this);
      }

    private:
      sdpa::job_id_list_t listJobIds_;

      friend class boost::serialization::access;
      template <class Archive>
      void serialize (Archive& ar, unsigned int)
      {
        ar & boost::serialization::base_object<MgmtEvent> (*this);
        ar & listJobIds_;
      }
    };
  }
}

#endif
