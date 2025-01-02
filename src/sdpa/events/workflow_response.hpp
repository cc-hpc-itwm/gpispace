// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/events/Serialization.hpp>

#include <we/type/value.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/serialize.hpp>

#include <util-generic/functor_visitor.hpp>
#include <util-generic/serialization/exception.hpp>
#include <util-generic/serialization/std/variant.hpp>

#include <boost/optional.hpp>

#include <exception>
#include <sstream>
#include <string>
#include <variant>

namespace sdpa
{
  namespace events
  {
    class workflow_response : public JobEvent
    {
    public:
      workflow_response ( job_id_t job_id
                        , std::string workflow_response_id
                        , std::string place_name
                        , pnet::type::value::value_type value
                        )
        : JobEvent (job_id)
        , _workflow_response_id (workflow_response_id)
        , _place_name (place_name)
        , _value (value)
      {}

      std::string const& workflow_response_id() const
      {
        return _workflow_response_id;
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
        handler->handle_workflow_response (source, this);
      }

    private:
      std::string _workflow_response_id;
      std::string _place_name;
      pnet::type::value::value_type _value;
    };

    SAVE_CONSTRUCT_DATA_DEF (workflow_response, e)
    {
      SAVE_JOBEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->workflow_response_id());
      SAVE_TO_ARCHIVE (e->place_name());
      SAVE_TO_ARCHIVE (e->value());
    }

    LOAD_CONSTRUCT_DATA_DEF (workflow_response, e)
    {
      LOAD_JOBEVENT_CONSTRUCT_DATA (job_id);
      LOAD_FROM_ARCHIVE (std::string, workflow_response_id);
      LOAD_FROM_ARCHIVE (std::string, place_name);
      LOAD_FROM_ARCHIVE (pnet::type::value::value_type, value);

      ::new (e) workflow_response ( job_id
                                  , workflow_response_id
                                  , place_name
                                  , value
                                  );
    }

    class workflow_response_response : public MgmtEvent
    {
    public:
      using value_t = pnet::type::value::value_type;
      using content_t = std::variant<std::exception_ptr, value_t>;

      workflow_response_response ( std::string workflow_response_id
                                 , content_t content
                                 )
        : MgmtEvent()
        , _workflow_response_id (workflow_response_id)
        , _content (std::move (content))
      {}

      std::string const& workflow_response_id() const
      {
        return _workflow_response_id;
      }

      value_t const& get() const
      {
        if (std::holds_alternative<value_t> (_content))
        {
          return std::get<value_t> (_content);
        }
        else
        {
          std::rethrow_exception (std::get<std::exception_ptr> (_content));
        }
      }

      void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handle_workflow_response_response (source, this);
      }

      //! \note for serialization only
      content_t const& content() const
      {
        return _content;
      }

      using serialized = std::variant< pnet::type::value::value_type
                                     , std::string
                                     >;

    private:
      std::string _workflow_response_id;
      content_t _content;
    };

    SAVE_CONSTRUCT_DATA_DEF (workflow_response_response, e)
    {
      SAVE_MGMTEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->workflow_response_id());

      SAVE_TO_ARCHIVE_WITH_TEMPORARY
        ( workflow_response_response::serialized
        , fhg::util::visit
          ( e->content()
          , [] ( workflow_response_response::value_t const& value
               ) -> workflow_response_response::serialized
            {
              return value;
            }
          , [] ( std::exception_ptr const& ex
               ) -> workflow_response_response::serialized
            {
              return fhg::util::serialization::exception::serialize (ex);
            }
          )
        )
    }

    LOAD_CONSTRUCT_DATA_DEF (workflow_response_response, e)
    {
      LOAD_MGMTEVENT_CONSTRUCT_DATA();
      LOAD_FROM_ARCHIVE (std::string, workflow_response_id);

      LOAD_FROM_ARCHIVE (workflow_response_response::serialized, content);

      ::new (e) workflow_response_response
          ( workflow_response_id
          , fhg::util::visit
            ( content
            , [] ( pnet::type::value::value_type const& value
                 ) -> workflow_response_response::content_t
              {
                return value;
              }
            , [] ( std::string const& ex
                 ) -> workflow_response_response::content_t
              {
                return fhg::util::serialization::exception::deserialize (ex);
              }
            )
          );
    }
  }
}
