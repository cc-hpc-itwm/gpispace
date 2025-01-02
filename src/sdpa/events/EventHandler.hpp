// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <fhgcom/address.hpp>

namespace sdpa
{
  namespace events
  {
    class CancelJobAckEvent;
    class CancelJobEvent;
    class DeleteJobAckEvent;
    class DeleteJobEvent;
    class ErrorEvent;
    class JobFailedAckEvent;
    class JobFailedEvent;
    class JobFinishedAckEvent;
    class JobFinishedEvent;
    class SubmitJobAckEvent;
    class SubmitJobEvent;
    class worker_registration_response;
    class WorkerRegistrationEvent;
    class SubscribeEvent;
    class SubscribeAckEvent;
    class put_token;
    class put_token_response;
    class workflow_response;
    class workflow_response_response;

    class EventHandler
    {
    public:
      virtual ~EventHandler() = default;

      EventHandler() = default;
      EventHandler (EventHandler const&) = delete;
      EventHandler& operator= (EventHandler const&) = delete;
      EventHandler (EventHandler&&) = delete;
      EventHandler& operator= (EventHandler&&) = delete;

#define HANDLER(Event, Sep)                             \
      virtual void handle ## Sep ## Event               \
        (fhg::com::p2p::address_t const&, const Event*)

      HANDLER (CancelJobAckEvent,);
      HANDLER (CancelJobEvent,);
      HANDLER (DeleteJobEvent,);
      HANDLER (ErrorEvent,);
      HANDLER (JobFailedAckEvent,);
      HANDLER (JobFailedEvent,);
      HANDLER (JobFinishedAckEvent,);
      HANDLER (JobFinishedEvent,);
      HANDLER (SubmitJobAckEvent,);
      HANDLER (SubmitJobEvent,);
      HANDLER (worker_registration_response,_);
      HANDLER (WorkerRegistrationEvent,);
      HANDLER (SubscribeEvent,);
      HANDLER (SubscribeAckEvent,);
      HANDLER (put_token,_);
      HANDLER (put_token_response,_);
      HANDLER (workflow_response,_);
      HANDLER (workflow_response_response,_);

#undef HANDLER
    };
  }
}
