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
#define REGISTER(TYPE, BASE)                                            \
        boost::serialization::void_cast_register<TYPE, BASE>();         \
        ar.template register_type<TYPE>()

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
