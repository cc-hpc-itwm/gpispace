// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#pragma once

#include <sdpa/events/MgmtEvent.hpp>

#include <util-generic/serialization/exception.hpp>

#include <boost/optional.hpp>

#include <exception>

namespace sdpa
{
  namespace events
  {
    class worker_registration_response : public MgmtEvent
    {
    public:
      worker_registration_response (::boost::optional<std::exception_ptr> error)
        : _error (std::move (error))
      {}

      void get() const
      {
        if (_error)
        {
          std::rethrow_exception (*_error);
        }
      }

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handle_worker_registration_response (source, this);
      }

      //! \note for serialization and TESTING only
      ::boost::optional<std::exception_ptr> const& exception() const
      {
        return _error;
      }

    private:
      ::boost::optional<std::exception_ptr> _error;
    };

    SAVE_CONSTRUCT_DATA_DEF (worker_registration_response, e)
    {
      SAVE_MGMTEVENT_CONSTRUCT_DATA (e);

      ::boost::optional<std::string> exception (::boost::none);
      if (!!e->exception())
      {
        exception = fhg::util::serialization::exception::serialize
          (e->exception().get());
      }
      SAVE_TO_ARCHIVE (exception);
    }

    LOAD_CONSTRUCT_DATA_DEF (worker_registration_response, e)
    {
      LOAD_MGMTEVENT_CONSTRUCT_DATA();
      LOAD_FROM_ARCHIVE (::boost::optional<std::string>, exception);
      if (!exception)
      {
        ::new (e) worker_registration_response (::boost::none);
      }
      else
      {
        ::new (e) worker_registration_response
          (fhg::util::serialization::exception::deserialize (exception.get()));
      }
    }
  }
}
