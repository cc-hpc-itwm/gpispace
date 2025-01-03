// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/events/Serialization.hpp>

#include <we/type/value.hpp>
#include <we/type/value/serialize.hpp>

#include <util-generic/serialization/exception.hpp>

#include <boost/optional.hpp>

#include <exception>
#include <string>

namespace sdpa
{
  namespace events
  {
    class put_token : public JobEvent
    {
    public:
      put_token ( job_id_t job_id
                , std::string put_token_id
                , std::string place_name
                , pnet::type::value::value_type value
                )
        : JobEvent (job_id)
        , _put_token_id (put_token_id)
        , _place_name (place_name)
        , _value (value)
      {}

      std::string const& put_token_id() const
      {
        return _put_token_id;
      }
      std::string const& place_name() const
      {
        return _place_name;
      }
      pnet::type::value::value_type const& value() const
      {
        return _value;
      }

      void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handle_put_token (source, this);
      }

    private:
      std::string _put_token_id;
      std::string _place_name;
      pnet::type::value::value_type _value;
    };

    SAVE_CONSTRUCT_DATA_DEF (put_token, e)
    {
      SAVE_JOBEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->put_token_id());
      SAVE_TO_ARCHIVE (e->place_name());
      SAVE_TO_ARCHIVE (e->value());
    }

    LOAD_CONSTRUCT_DATA_DEF (put_token, e)
    {
      LOAD_JOBEVENT_CONSTRUCT_DATA (job_id);
      LOAD_FROM_ARCHIVE (std::string, put_token_id);
      LOAD_FROM_ARCHIVE (std::string, place_name);
      LOAD_FROM_ARCHIVE (pnet::type::value::value_type, value);

      ::new (e) put_token ( job_id
                          , put_token_id
                          , place_name
                          , value
                          );
    }

    class put_token_response : public MgmtEvent
    {
    public:
      put_token_response ( std::string put_token_id
                         , ::boost::optional<std::exception_ptr> error
                         )
        : MgmtEvent()
        , _put_token_id (put_token_id)
        , _error (std::move (error))
      {}

      std::string const& put_token_id() const
      {
        return _put_token_id;
      }

      void get() const
      {
        if (_error)
        {
          std::rethrow_exception (*_error);
        }
      }

      void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handle_put_token_response (source, this);
      }

      //! \note for serialization only
      ::boost::optional<std::exception_ptr> const& exception() const
      {
        return _error;
      }

    private:
      std::string _put_token_id;
      ::boost::optional<std::exception_ptr> _error;
    };

    SAVE_CONSTRUCT_DATA_DEF (put_token_response, e)
    {
      SAVE_MGMTEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->put_token_id());

      ::boost::optional<std::string> exception (::boost::none);
      if (!!e->exception())
      {
        exception = fhg::util::serialization::exception::serialize
          (e->exception().get());
      }
      SAVE_TO_ARCHIVE (exception);
    }

    LOAD_CONSTRUCT_DATA_DEF (put_token_response, e)
    {
      LOAD_MGMTEVENT_CONSTRUCT_DATA();
      LOAD_FROM_ARCHIVE (std::string, put_token_id);
      LOAD_FROM_ARCHIVE (::boost::optional<std::string>, exception);
      if (!exception)
      {
        ::new (e) put_token_response (put_token_id, ::boost::none);
      }
      else
      {
        ::new (e) put_token_response
          ( put_token_id
          , fhg::util::serialization::exception::deserialize (exception.get())
          );
      }
    }
  }
}
