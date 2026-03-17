// Copyright (C) 2019-2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/scheduler/events/Codec.hpp>

#include <gspc/scheduler/events/CancelJobAckEvent.hpp>
#include <gspc/scheduler/events/CancelJobEvent.hpp>
#include <gspc/scheduler/events/DeleteJobAckEvent.hpp>
#include <gspc/scheduler/events/DeleteJobEvent.hpp>
#include <gspc/scheduler/events/ErrorEvent.hpp>
#include <gspc/scheduler/events/JobFailedAckEvent.hpp>
#include <gspc/scheduler/events/JobFailedEvent.hpp>
#include <gspc/scheduler/events/JobFinishedAckEvent.hpp>
#include <gspc/scheduler/events/JobFinishedEvent.hpp>
#include <gspc/scheduler/events/Serialization.hpp>
#include <gspc/scheduler/events/SubmitJobAckEvent.hpp>
#include <gspc/scheduler/events/SubmitJobEvent.hpp>
#include <gspc/scheduler/events/SubscribeAckEvent.hpp>
#include <gspc/scheduler/events/SubscribeEvent.hpp>
#include <gspc/scheduler/events/WorkerRegistrationEvent.hpp>
#include <gspc/scheduler/events/put_token.hpp>
#include <gspc/scheduler/events/worker_registration_response.hpp>
#include <gspc/scheduler/events/workflow_response.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <sstream>


  namespace gspc::scheduler::events
  {
    namespace
    {
      template <class Archive>
        void initialize_archive (Archive& ar)
      {
        ::boost::serialization::void_cast_register<JobEvent, SchedulerEvent>();
        ::boost::serialization::void_cast_register<MgmtEvent, SchedulerEvent>();

#define REGISTER(TYPE, BASE)                                            \
        ::boost::serialization::void_cast_register<TYPE, BASE>();         \
        ar.template register_type<TYPE>()

        REGISTER (CancelJobAckEvent, JobEvent);
        REGISTER (CancelJobEvent, JobEvent);
        REGISTER (DeleteJobAckEvent, JobEvent);
        REGISTER (DeleteJobEvent, JobEvent);
        REGISTER (ErrorEvent, MgmtEvent);
        REGISTER (JobFailedAckEvent, JobEvent);
        REGISTER (JobFailedEvent, JobEvent);
        REGISTER (JobFinishedAckEvent, JobEvent);
        REGISTER (JobFinishedEvent, JobEvent);
        REGISTER (SubmitJobAckEvent, JobEvent);
        REGISTER (SubmitJobEvent, SchedulerEvent);
        REGISTER (SubscribeAckEvent, MgmtEvent);
        REGISTER (SubscribeEvent, MgmtEvent);
        REGISTER (worker_registration_response, MgmtEvent);
        REGISTER (WorkerRegistrationEvent, MgmtEvent);
        REGISTER (put_token, JobEvent);
        REGISTER (put_token_response, MgmtEvent);
        REGISTER (workflow_response, JobEvent);
        REGISTER (workflow_response_response, MgmtEvent);

#undef REGISTER

      }
    }

    std::string Codec::encode (gspc::scheduler::events::SchedulerEvent const* e) const
    {
      std::ostringstream sstr;
      ::boost::archive::text_oarchive ar (sstr);
      initialize_archive (ar);
      ar << e;
      return sstr.str();
    }

    gspc::scheduler::events::SchedulerEvent* Codec::decode (std::string const& s) const
    {
      std::istringstream sstr (s);
      ::boost::archive::text_iarchive ar (sstr);
      initialize_archive (ar);
      SchedulerEvent* e (nullptr);
      ar >> e;
      return e;
    }
  }
