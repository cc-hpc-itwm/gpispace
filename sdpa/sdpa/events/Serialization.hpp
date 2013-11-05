#ifndef SDPA_EVENTS_SERIALIZATION_HPP
#define SDPA_EVENTS_SERIALIZATION_HPP 1

#include <string>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/assume_abstract.hpp>

#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/events/LifeSignEvent.hpp>
#include <sdpa/events/RequestJobEvent.hpp>
#include <sdpa/events/WorkerRegistrationEvent.hpp>
#include <sdpa/events/WorkerRegistrationAckEvent.hpp>

#include <sdpa/events/SubmitJobEvent.hpp>
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/DeleteJobEvent.hpp>
#include <sdpa/events/DeleteJobAckEvent.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/JobFailedEvent.hpp>
#include <sdpa/events/JobFailedAckEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/JobFinishedAckEvent.hpp>
#include <sdpa/events/RetrieveJobResultsEvent.hpp>
#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/QueryJobStatusEvent.hpp>
#include <sdpa/events/JobStatusReplyEvent.hpp>
#include <sdpa/events/CapabilitiesGainedEvent.hpp>
#include <sdpa/events/CapabilitiesLostEvent.hpp>
#include <sdpa/events/SubscribeEvent.hpp>
#include <sdpa/events/SubscribeAckEvent.hpp>
#include <boost/serialization/set.hpp>


namespace boost
{
  namespace serialization
  {
    BOOST_SERIALIZATION_ASSUME_ABSTRACT (sdpa::events::SDPAEvent)
    BOOST_SERIALIZATION_ASSUME_ABSTRACT (sdpa::events::JobEvent)
    BOOST_SERIALIZATION_ASSUME_ABSTRACT (sdpa::events::MgmtEvent)
  }
}

#endif
