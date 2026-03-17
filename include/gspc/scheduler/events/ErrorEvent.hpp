// Copyright (C) 2010-2011,2013-2015,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/scheduler/events/MgmtEvent.hpp>
#include <gspc/scheduler/events/Serialization.hpp>
#include <gspc/scheduler/types.hpp>

#include <optional>


  namespace gspc::scheduler::events
  {
    class ErrorEvent : public MgmtEvent
    {
    public:
      using Ptr = std::shared_ptr<ErrorEvent>;

      enum error_code_t
        {
          SCHEDULER_ENODE_SHUTDOWN,
          SCHEDULER_EUNKNOWN,
        };

      ErrorEvent
        ( error_code_t const& a_error_code
        , std::string const& a_reason
        //! \todo This should not be in _every_ ErrorEvent!
        , std::optional<gspc::scheduler::job_id_t> const& jobId = std::nullopt
        )
          : MgmtEvent()
          , error_code_ (a_error_code)
          , reason_ (a_reason)
          , job_id_ (jobId)
      {}

      std::string const& reason() const
      {
        return reason_;
      }
      error_code_t const& error_code() const
      {
        return error_code_;
      }
      std::optional<gspc::scheduler::job_id_t> const& job_id() const
      {
        return job_id_;
      }

      void handleBy
        (gspc::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleErrorEvent (source, this);
      }

    private:
      //! \todo Rename to leading-underscore convention.
      error_code_t error_code_;
      std::string reason_;
      std::optional<gspc::scheduler::job_id_t> job_id_;
    };

    SAVE_CONSTRUCT_DATA_DEF (ErrorEvent, e)
    {
      SAVE_MGMTEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->error_code());
      SAVE_TO_ARCHIVE (e->reason());
      SAVE_TO_ARCHIVE (e->job_id());
    }

    LOAD_CONSTRUCT_DATA_DEF (ErrorEvent, e)
    {
      LOAD_MGMTEVENT_CONSTRUCT_DATA();
      LOAD_FROM_ARCHIVE (ErrorEvent::error_code_t, error_code);
      LOAD_FROM_ARCHIVE (std::string, reason);
      LOAD_FROM_ARCHIVE (std::optional<gspc::scheduler::job_id_t>, job_id);

      ::new (e) ErrorEvent (error_code, reason, job_id);
    }
  }
