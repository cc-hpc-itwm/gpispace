// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <sdpa/events/Codec.hpp>

#include <sdpa/events/BacklogNoLongerFullEvent.hpp>
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
#include <sdpa/events/JobStatusReplyEvent.hpp>
#include <sdpa/events/Serialization.hpp>
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/SubmitJobEvent.hpp>
#include <sdpa/events/SubscribeAckEvent.hpp>
#include <sdpa/events/SubscribeEvent.hpp>
#include <sdpa/events/WorkerRegistrationEvent.hpp>
#include <sdpa/events/put_token.hpp>
#include <sdpa/events/worker_registration_response.hpp>
#include <sdpa/events/workflow_response.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <sstream>

namespace sdpa
{
  namespace events
  {
    namespace
    {
      template <class Archive>
        void initialize_archive (Archive& ar)
      {
        boost::serialization::void_cast_register<JobEvent, SDPAEvent>();
        boost::serialization::void_cast_register<MgmtEvent, SDPAEvent>();

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
        REGISTER (JobStatusReplyEvent, JobEvent);
        REGISTER (SubmitJobAckEvent, JobEvent);
        REGISTER (SubmitJobEvent, SDPAEvent);
        REGISTER (SubscribeAckEvent, MgmtEvent);
        REGISTER (SubscribeEvent, MgmtEvent);
        REGISTER (worker_registration_response, MgmtEvent);
        REGISTER (WorkerRegistrationEvent, MgmtEvent);
        REGISTER (put_token, JobEvent);
        REGISTER (put_token_response, MgmtEvent);
        REGISTER (workflow_response, JobEvent);
        REGISTER (workflow_response_response, MgmtEvent);
        REGISTER (BacklogNoLongerFullEvent, MgmtEvent);

#undef REGISTER

      }
    }

    std::string Codec::encode (sdpa::events::SDPAEvent const* e) const
    {
      std::ostringstream sstr;
      boost::archive::text_oarchive ar (sstr);
      initialize_archive (ar);
      ar << e;
      return sstr.str();
    }

    sdpa::events::SDPAEvent* Codec::decode (std::string const& s) const
    {
      std::istringstream sstr (s);
      boost::archive::text_iarchive ar (sstr);
      initialize_archive (ar);
      SDPAEvent* e (nullptr);
      ar >> e;
      return e;
    }
  }
}
