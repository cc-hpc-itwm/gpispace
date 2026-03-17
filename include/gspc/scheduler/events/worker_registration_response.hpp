// Copyright (C) 2015-2016,2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/scheduler/events/MgmtEvent.hpp>

#include <gspc/util/serialization/exception.hpp>

#include <optional>

#include <exception>


  namespace gspc::scheduler::events
  {
    class worker_registration_response : public MgmtEvent
    {
    public:
      worker_registration_response (std::optional<std::exception_ptr> error)
        : _error (std::move (error))
      {}

      void get() const
      {
        if (_error)
        {
          std::rethrow_exception (*_error);
        }
      }

      void handleBy
        (gspc::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handle_worker_registration_response (source, this);
      }

      //! \note for serialization and TESTING only
      std::optional<std::exception_ptr> const& exception() const
      {
        return _error;
      }

    private:
      std::optional<std::exception_ptr> _error;
    };

    SAVE_CONSTRUCT_DATA_DEF (worker_registration_response, e)
    {
      SAVE_MGMTEVENT_CONSTRUCT_DATA (e);

      std::optional<std::string> exception (std::nullopt);
      if (!!e->exception())
      {
        exception = gspc::util::serialization::exception::serialize
          (e->exception().value());
      }
      SAVE_TO_ARCHIVE (exception);
    }

    LOAD_CONSTRUCT_DATA_DEF (worker_registration_response, e)
    {
      LOAD_MGMTEVENT_CONSTRUCT_DATA();
      LOAD_FROM_ARCHIVE (std::optional<std::string>, exception);
      if (!exception)
      {
        ::new (e) worker_registration_response (std::nullopt);
      }
      else
      {
        ::new (e) worker_registration_response
          (gspc::util::serialization::exception::deserialize (exception.value()));
      }
    }
  }
