#ifndef SDPA_ConfigReplyEvent_HPP
#define SDPA_ConfigReplyEvent_HPP 1

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/MgmtEvent.hpp>

#include <boost/serialization/base_object.hpp>

namespace sdpa
{
  namespace events
  {
    class ConfigReplyEvent : public MgmtEvent
    {
    public:
      typedef sdpa::shared_ptr<ConfigReplyEvent> Ptr;

      ConfigReplyEvent()
        : MgmtEvent()
      {}

      ConfigReplyEvent ( const address_t& from
                       , const address_t& to
                       )
        : MgmtEvent(from, to)
      {}

      std::string str() const
      {
        return "ConfigReplyEvent";
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleConfigReplyEvent (this);
      }

    private:
      friend class boost::serialization::access;
      template <typename Archive>
      void serialize (Archive& ar, const unsigned int)
      {
        ar & boost::serialization::base_object<MgmtEvent> (*this);
      }
    };
  }
}

#endif
