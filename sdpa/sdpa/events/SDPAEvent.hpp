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

      virtual ~SDPAEvent() = default;

      virtual void handleBy (std::string const& source, EventHandler*) = 0;

    protected:
      SDPAEvent() = default;

    private:
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
