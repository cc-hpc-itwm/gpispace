#ifndef SDPA_EVENT_HPP
#define SDPA_EVENT_HPP 1

#include <sdpa/events/EventHandler.hpp>

#include <boost/serialization/access.hpp>
#include <boost/shared_ptr.hpp>

#include <string>

namespace sdpa
{
  namespace events
  {
    class SDPAEvent
    {
    public:
      typedef boost::shared_ptr<SDPAEvent> Ptr;

      typedef std::string address_t;

      virtual ~SDPAEvent() = default;

      const address_t& from() const
      {
        return from_;
      }

      virtual void handleBy (EventHandler*) = 0;

    protected:
      SDPAEvent (const address_t & a_from)
        : from_ (a_from)
      {}

    private:
      address_t from_;

      friend class boost::serialization::access;
      template <class Archive>
      void serialize (Archive &, unsigned int)
      {
      }
    };
  }
}

#include <sdpa/events/Serialization.hpp>

#endif
