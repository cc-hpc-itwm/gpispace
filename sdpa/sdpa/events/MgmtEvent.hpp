#ifndef SDPA_MGMT_EVENT_HPP
#define SDPA_MGMT_EVENT_HPP 1

#include <string>

#include <sdpa/memory.hpp>
#include <sdpa/events/SDPAEvent.hpp>

#include <boost/serialization/base_object.hpp>

namespace sdpa
{
  namespace events
  {
    class MgmtEvent : public sdpa::events::SDPAEvent
    {
    public:
      typedef sdpa::shared_ptr<MgmtEvent> Ptr;

      MgmtEvent()
        : SDPAEvent()
      {}
      MgmtEvent (const address_t &a_from, const address_t &a_to)
        : SDPAEvent (a_from, a_to)
      {}
      MgmtEvent ( const address_t &a_from
                , const address_t &a_to
                , SDPAEvent::message_id_type const& id
                )
        : SDPAEvent (a_from, a_to, id)
      {}

      int priority() const
      {
        return 1;
      }

      virtual std::string str() const = 0;

    private:
      friend class boost::serialization::access;
      template <typename Archive>
      void serialize (Archive& ar, const unsigned int)
      {
        ar & boost::serialization::base_object<SDPAEvent> (*this);
      }
    };
  }
}

#endif
