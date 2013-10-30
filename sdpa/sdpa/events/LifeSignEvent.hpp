#ifndef SDPA_LIFESIGNEVENT_HPP
#define SDPA_LIFESIGNEVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/types.hpp>

#include <boost/serialization/base_object.hpp>

namespace sdpa
{
  namespace events
  {
    class LifeSignEvent : public MgmtEvent
    {
    public:
      typedef sdpa::shared_ptr<LifeSignEvent> Ptr;

      LifeSignEvent()
        : MgmtEvent()
        , last_job_id_ ("")
      {}

      LifeSignEvent ( const address_t& from
                    , const address_t& to
                    , const sdpa::job_id_t &the_last_job = ""
                    )
        : MgmtEvent (from, to)
        , last_job_id_ (the_last_job)
      {}

      const sdpa::job_id_t& last_job_id() const
      {
        return last_job_id_;
      }

      std::string str() const
      {
        return "LifeSignEvent";
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleLifeSignEvent (this);
      }

    private:
      sdpa::job_id_t last_job_id_;

      friend class boost::serialization::access;
      template <typename Archive>
      void serialize (Archive& ar, const unsigned int)
      {
        ar & boost::serialization::base_object<MgmtEvent> (*this);
        ar & last_job_id_;
      }
    };
  }
}

#endif
