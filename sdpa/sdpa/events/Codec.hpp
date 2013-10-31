#ifndef SDPA_EVENTS_CODEC_HPP
#define SDPA_EVENTS_CODEC_HPP 1

#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sdpa/events/Serialization.hpp>

namespace sdpa
{
  namespace events
  {
    class Codec
    {
    public:

      std::string encode (sdpa::events::SDPAEvent* e) const
      {
        std::ostringstream sstr;
        boost::archive::text_oarchive ar (sstr);
        initialize_archive (ar);
        ar << e;
        return sstr.str();
      }

      sdpa::events::SDPAEvent* decode (const std::string& s) const
      {
        std::istringstream sstr (s);
        boost::archive::text_iarchive ar (sstr);
        initialize_archive (ar);
        SDPAEvent* e (NULL);
        ar >> e;
        return e;
      }

    private:
      template <class Archive>
      void initialize_archive (Archive & ar) const
      {
        ar.register_type (static_cast<CancelJobAckEvent*>(NULL));
        ar.register_type (static_cast<CancelJobEvent*>(NULL));
        ar.register_type (static_cast<ConfigReplyEvent*>(NULL));
        ar.register_type (static_cast<ConfigRequestEvent*>(NULL));
        ar.register_type (static_cast<DeleteJobAckEvent*>(NULL));
        ar.register_type (static_cast<DeleteJobEvent*>(NULL));
        ar.register_type (static_cast<ErrorEvent*>(NULL));
        ar.register_type (static_cast<JobFailedAckEvent*>(NULL));
        ar.register_type (static_cast<JobFailedEvent*>(NULL));
        ar.register_type (static_cast<JobFinishedAckEvent*>(NULL));
        ar.register_type (static_cast<JobFinishedEvent*>(NULL));
        ar.register_type (static_cast<JobResultsReplyEvent*>(NULL));
        ar.register_type (static_cast<JobStatusReplyEvent*>(NULL));
        ar.register_type (static_cast<LifeSignEvent*>(NULL));
        ar.register_type (static_cast<QueryJobStatusEvent*>(NULL));
        ar.register_type (static_cast<RequestJobEvent*>(NULL));
        ar.register_type (static_cast<RetrieveJobResultsEvent*>(NULL));
        ar.register_type (static_cast<SubmitJobAckEvent*>(NULL));
        ar.register_type (static_cast<SubmitJobEvent*>(NULL));
        ar.register_type (static_cast<WorkerRegistrationAckEvent*>(NULL));
        ar.register_type (static_cast<WorkerRegistrationEvent*>(NULL));
        ar.register_type (static_cast<CapabilitiesGainedEvent*>(NULL));
        ar.register_type (static_cast<CapabilitiesLostEvent*>(NULL));
        ar.register_type (static_cast<SubscribeEvent*>(NULL));
        ar.register_type (static_cast<SubscribeAckEvent*>(NULL));
      }
    };
  }
}

#endif
