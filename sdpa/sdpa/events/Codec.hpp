#ifndef SDPA_EVENTS_CODEC_HPP
#define SDPA_EVENTS_CODEC_HPP 1

#include <sdpa/events/DiscoverPendingActEvent.hpp>
#include <sdpa/events/DiscoverPendingActReplyEvent.hpp>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/CapabilitiesGainedEvent.hpp>
#include <sdpa/events/CapabilitiesLostEvent.hpp>
#include <sdpa/events/DeleteJobAckEvent.hpp>
#include <sdpa/events/DeleteJobEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/events/JobFailedAckEvent.hpp>
#include <sdpa/events/JobFailedEvent.hpp>
#include <sdpa/events/JobFinishedAckEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/JobRunningEvent.hpp>
#include <sdpa/events/JobStalledEvent.hpp>
#include <sdpa/events/JobStatusReplyEvent.hpp>
#include <sdpa/events/QueryJobStatusEvent.hpp>
#include <sdpa/events/RetrieveJobResultsEvent.hpp>
#include <sdpa/events/Serialization.hpp>
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/SubmitJobEvent.hpp>
#include <sdpa/events/SubscribeAckEvent.hpp>
#include <sdpa/events/SubscribeEvent.hpp>
#include <sdpa/events/WorkerRegistrationAckEvent.hpp>
#include <sdpa/events/WorkerRegistrationEvent.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <sstream>

namespace sdpa
{
  namespace events
  {
    class Codec
    {
    public:

      std::string encode (const sdpa::events::SDPAEvent* e) const
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
        boost::serialization::void_cast_register<JobEvent, SDPAEvent>();
        boost::serialization::void_cast_register<MgmtEvent, SDPAEvent>();

#define REGISTER(TYPE, BASE)                                            \
        boost::serialization::void_cast_register<TYPE, BASE>();         \
        ar.template register_type<TYPE>()

        REGISTER (DiscoverPendingActEvent, JobEvent);
        REGISTER (DiscoverPendingActReplyEvent, JobEvent);
        REGISTER (CancelJobAckEvent, JobEvent);
        REGISTER (CancelJobEvent, JobEvent);
        REGISTER (CapabilitiesGainedEvent, MgmtEvent);
        REGISTER (CapabilitiesLostEvent, MgmtEvent);
        REGISTER (DeleteJobAckEvent, JobEvent);
        REGISTER (DeleteJobEvent, JobEvent);
        REGISTER (ErrorEvent, MgmtEvent);
        REGISTER (JobFailedAckEvent, JobEvent);
        REGISTER (JobFailedEvent, JobEvent);
        REGISTER (JobFinishedAckEvent, JobEvent);
        REGISTER (JobFinishedEvent, JobEvent);
        REGISTER (JobResultsReplyEvent, JobEvent);
        REGISTER (JobRunningEvent, JobEvent);
        REGISTER (JobStalledEvent, JobEvent);
        REGISTER (JobStatusReplyEvent, JobEvent);
        REGISTER (QueryJobStatusEvent, JobEvent);
        REGISTER (RetrieveJobResultsEvent, JobEvent);
        REGISTER (SubmitJobAckEvent, JobEvent);
        REGISTER (SubmitJobEvent, JobEvent);
        REGISTER (SubscribeAckEvent, MgmtEvent);
        REGISTER (SubscribeEvent, MgmtEvent);
        REGISTER (WorkerRegistrationAckEvent, MgmtEvent);
        REGISTER (WorkerRegistrationEvent, MgmtEvent);

#undef REGISTER

      }
    };
  }
}

#endif
