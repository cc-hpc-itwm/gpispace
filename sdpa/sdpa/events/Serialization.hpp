#ifndef SDPA_EVENTS_SERIALIZATION_HPP
#define SDPA_EVENTS_SERIALIZATION_HPP 1

#define SAVE_TO_ARCHIVE(WHAT)                   \
  _ARCHIVE << WHAT

#define LOAD_FROM_ARCHIVE(TYPE, VARIABLE_NAME)  \
  TYPE VARIABLE_NAME; _ARCHIVE >> VARIABLE_NAME


#define SAVE_CONSTRUCT_DATA_DEF(TYPE, VARIABLE_NAME)                    \
  template<class Archive> inline void save_construct_data               \
    (Archive& _ARCHIVE, const TYPE* VARIABLE_NAME, const unsigned int)

#define LOAD_CONSTRUCT_DATA_DEF(TYPE, VARIABLE_NAME)              \
  template<class Archive> inline void load_construct_data         \
    (Archive& _ARCHIVE, TYPE* VARIABLE_NAME, const unsigned int)


#define SAVE_SDPAEVENT_CONSTRUCT_DATA(EVENT_VARIABLE)   \
  SAVE_TO_ARCHIVE (EVENT_VARIABLE->from());             \
  SAVE_TO_ARCHIVE (EVENT_VARIABLE->to())

#define LOAD_SDPAEVENT_CONSTRUCT_DATA(FROM_VAR_NAME, TO_VAR_NAME)        \
  LOAD_FROM_ARCHIVE (sdpa::events::SDPAEvent::address_t, FROM_VAR_NAME); \
  LOAD_FROM_ARCHIVE (sdpa::events::SDPAEvent::address_t, TO_VAR_NAME)


#define SAVE_JOBEVENT_CONSTRUCT_DATA(EVENT_VARIABLE)    \
  SAVE_SDPAEVENT_CONSTRUCT_DATA (EVENT_VARIABLE);       \
  SAVE_TO_ARCHIVE (EVENT_VARIABLE->job_id())

#define LOAD_JOBEVENT_CONSTRUCT_DATA(FROM_VAR_NAME, TO_VAR_NAME, JOB_ID_VAR_NAME) \
  LOAD_SDPAEVENT_CONSTRUCT_DATA (FROM_VAR_NAME, TO_VAR_NAME);           \
  LOAD_FROM_ARCHIVE (sdpa::job_id_t, JOB_ID_VAR_NAME)

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
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/SubmitJobEvent.hpp>
#include <sdpa/events/SubscribeAckEvent.hpp>
#include <sdpa/events/SubscribeEvent.hpp>
#include <sdpa/events/WorkerRegistrationAckEvent.hpp>
#include <sdpa/events/WorkerRegistrationEvent.hpp>

#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/utility.hpp>

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
