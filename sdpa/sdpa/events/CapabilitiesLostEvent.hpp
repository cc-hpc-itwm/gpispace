#ifndef SDPA_CAPABILITIES_LOST_EVENT
#define SDPA_CAPABILITIES_LOST_EVENT 1

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/capability.hpp>

#include <boost/serialization/base_object.hpp>

namespace sdpa
{
  namespace events
  {
    class CapabilitiesLostEvent : public MgmtEvent
    {
    public:
      typedef sdpa::shared_ptr<CapabilitiesLostEvent> Ptr;

      CapabilitiesLostEvent()
        : MgmtEvent()
      {}

      CapabilitiesLostEvent
        ( const address_t& from
        , const address_t& to
        , const sdpa::capabilities_set_t& cpbs = capabilities_set_t()
        )
          : MgmtEvent (from, to)
          , capabilities_ (cpbs)
      {}

      CapabilitiesLostEvent ( const address_t& from
                            , const address_t& to
                            , const sdpa::capability_t &cap
                            )
        : MgmtEvent(from, to)
        , capabilities_()
      {
        capabilities_.insert (cap);
      }

      const sdpa::capabilities_set_t& capabilities() const
      {
        return capabilities_;
      }
      std::string str() const
      {
        return "CapabilitiesLostEvent";
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleCapabilitiesLostEvent (this);
      }

    private:
      sdpa::capabilities_set_t capabilities_;

      friend class boost::serialization::access;
      template <class Archive>
      void serialize (Archive & ar, unsigned int)
      {
        ar & boost::serialization::base_object<MgmtEvent> (*this);
        ar & capabilities_;
      }
    };
  }
}

#endif
