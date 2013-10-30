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
#include <sdpa/events/ConfigRequestEvent.hpp>
#include <sdpa/events/ConfigReplyEvent.hpp>
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


namespace boost { namespace serialization {
  template <class Archive>
  void serialize(Archive & ar, sdpa::events::SDPAEvent & e, unsigned int /* version */)
  {
	  ar & e.from();
	  ar & e.to();
	  ar & e.id();
  }

  // ***********
  // Mgmt events
  // ***********
  template <class Archive>
  void serialize(Archive & ar, sdpa::events::MgmtEvent & e, unsigned int /* version */)
  {
	  ar & boost::serialization::base_object<sdpa::events::SDPAEvent>(e);
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::ErrorEvent & e, unsigned int /* version */)
  {
	  ar & boost::serialization::base_object<sdpa::events::MgmtEvent>(e);
	  ar & e.error_code();
	  ar & e.reason();
	  ar & e.job_id();
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::ConfigReplyEvent & e, unsigned int /* version */)
  {
	  ar & boost::serialization::base_object<sdpa::events::MgmtEvent>(e);
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::ConfigRequestEvent & e, unsigned int /* version */)
  {
	  ar & boost::serialization::base_object<sdpa::events::MgmtEvent>(e);
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::LifeSignEvent & e, unsigned int /* version */)
  {
      ar & boost::serialization::base_object<sdpa::events::MgmtEvent>(e);
      ar & e.last_job_id();
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::WorkerRegistrationEvent & e, unsigned int /* version */)
  {
      ar & boost::serialization::base_object<sdpa::events::MgmtEvent>(e);
      ar & e.capacity();
      ar & e.capabilities();
      ar & e.rank();
      ar & e.agent_uuid();
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::WorkerRegistrationAckEvent & e, unsigned int /* version */)
  {
	  ar & boost::serialization::base_object<sdpa::events::MgmtEvent>(e);
  }

  // ***********
  // Job events
  // ***********

  template <class Archive>
  void serialize(Archive & ar, sdpa::job_id_t & job_id, unsigned int /* version */)
  {
	  ar & job_id.str();
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::JobEvent & e, unsigned int /* version */)
  {
	  ar & boost::serialization::base_object<sdpa::events::SDPAEvent>(e);
	  ar & e.job_id();
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::SubmitJobEvent & e, unsigned int /* version */)
  {
	  ar & boost::serialization::base_object<sdpa::events::JobEvent>(e);
	  ar & e.description();
	  ar & e.parent_id();
	  ar & e.worker_list();
  }
  template <class Archive>
  void serialize(Archive & ar, sdpa::events::SubmitJobAckEvent & e, unsigned int /* version */)
  {
	  ar & boost::serialization::base_object<sdpa::events::JobEvent>(e);
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::DeleteJobEvent & e, unsigned int /* version */)
  {
	  ar & boost::serialization::base_object<sdpa::events::JobEvent>(e);
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::DeleteJobAckEvent & e, unsigned int /* version */)
  {
	  ar & boost::serialization::base_object<sdpa::events::JobEvent>(e);
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::JobFailedEvent & e, unsigned int /* version */)
  {
	  ar & boost::serialization::base_object<sdpa::events::JobEvent>(e);
	  ar & e.result();
	  ar & e.error_code();
	  ar & e.error_message();
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::JobFailedAckEvent & e, unsigned int /* version */)
  {
    ar & boost::serialization::base_object<sdpa::events::JobEvent>(e);
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::JobFinishedEvent & e, unsigned int /* version */)
  {
    ar & boost::serialization::base_object<sdpa::events::JobEvent>(e);
    ar & e.result();
  }
  template <class Archive>
  void serialize(Archive & ar, sdpa::events::JobFinishedAckEvent & e, unsigned int /* version */)
  {
    ar & boost::serialization::base_object<sdpa::events::JobEvent>(e);
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::RetrieveJobResultsEvent & e, unsigned int /* version */)
  {
    ar & boost::serialization::base_object<sdpa::events::JobEvent>(e);
  }
  template <class Archive>
  void serialize(Archive & ar, sdpa::events::JobResultsReplyEvent & e, unsigned int /* version */)
  {
	  ar & boost::serialization::base_object<sdpa::events::JobEvent>(e);
	  ar & e.result();
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::QueryJobStatusEvent & e, unsigned int /* version */)
  {
	  ar & boost::serialization::base_object<sdpa::events::JobEvent>(e);
  }
  template <class Archive>
  void serialize(Archive & ar, sdpa::events::JobStatusReplyEvent & e, unsigned int /* version */)
  {
	  ar & boost::serialization::base_object<sdpa::events::JobEvent>(e);
	  ar & e.status();
	  ar & e.error_code();
	  ar & e.error_message();
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::CapabilitiesGainedEvent & e, unsigned int /* version */)
  {
	  ar & boost::serialization::base_object<sdpa::events::MgmtEvent>(e);
      ar & e.capabilities();
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::CapabilitiesLostEvent & e, unsigned int /* version */)
  {
	  ar & boost::serialization::base_object<sdpa::events::MgmtEvent>(e);
      ar & e.capabilities();
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::SubscribeEvent & e, unsigned int /* version */)
  {
	  ar & boost::serialization::base_object<sdpa::events::MgmtEvent>(e);
      ar & e.subscriber();
      ar & e.listJobIds();
  }

  template <class Archive>
  void serialize(Archive & ar, sdpa::events::SubscribeAckEvent & e, unsigned int /* version */)
  {
      ar & boost::serialization::base_object<sdpa::events::MgmtEvent>(e);
      ar & e.listJobIds();
  }

  BOOST_SERIALIZATION_ASSUME_ABSTRACT(sdpa::events::SDPAEvent)
  BOOST_SERIALIZATION_ASSUME_ABSTRACT(sdpa::events::JobEvent)
  BOOST_SERIALIZATION_ASSUME_ABSTRACT(sdpa::events::MgmtEvent)

}}

#endif
