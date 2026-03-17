// Copyright (C) 2021,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/scheduler/events/EventHandler.hpp>

#include <gspc/scheduler/events/ErrorEvent.hpp>
#include <gspc/scheduler/events/JobFailedEvent.hpp>
#include <gspc/scheduler/events/WorkerRegistrationEvent.hpp>
#include <gspc/scheduler/events/put_token.hpp>
#include <gspc/scheduler/events/workflow_response.hpp>

#include <gspc/we/type/value/show.hpp>

#include <gspc/util/functor_visitor.hpp>
#include <gspc/util/print_container.hpp>
#include <gspc/util/print_exception.hpp>

#include <gspc/scheduler/events/error_code.formatter.hpp>
#include <gspc/util/join.formatter.hpp>
#include <gspc/we/type/value/show.formatter.hpp>
#include <fmt/core.h>
#include <stdexcept>
#include <string>


  namespace gspc::scheduler::events
  {
    namespace detail
    {
      struct semi{};
    }

#define UNHANDLED(Event, Sep, Var, Message...)                        \
    void EventHandler::handle ## Sep ## Event                         \
      (gspc::com::p2p::address_t const&, const Event* Var)             \
    {                                                                 \
      throw std::runtime_error                                        \
        {fmt::format ("{}{}", #Event, Message)};                      \
    } using ::gspc::scheduler::events::detail::semi

    UNHANDLED (CancelJobAckEvent,,,"");

    UNHANDLED (CancelJobEvent,,,"");

    UNHANDLED (DeleteJobEvent,,,"");

    UNHANDLED
      ( ErrorEvent
      ,
      , e
      , fmt::format ( ": error_code '{}', reason '{}'"
                    , e->error_code()
                    , e->reason()
                    )
      );

    UNHANDLED (JobFailedAckEvent,,,"");

    UNHANDLED
      ( JobFailedEvent
      ,
      , f
      , fmt::format (": error_message '{}'",  f->error_message())
      );

    UNHANDLED (JobFinishedAckEvent,,,"");

    UNHANDLED (JobFinishedEvent,,,"");

    UNHANDLED (SubmitJobAckEvent,,,"");

    UNHANDLED (SubmitJobEvent,,,"");

    UNHANDLED (SubscribeAckEvent,,,"");

    UNHANDLED (SubscribeEvent,,,"");

    UNHANDLED
      ( WorkerRegistrationEvent
      ,
      , w
      , fmt::format
        ( "name '{}', host '{}', capabilites '{}', shm_size '{}'"
        , w->name()
        , w->hostname()
        , gspc::util::print_container
            ( "{", ", ", "}", w->capabilities()
            , [] (auto& os, auto const& capability) -> decltype (os)
              {
                return os << capability.name();
              }
            )
        , w->allocated_shared_memory_size()
        )
      );

    UNHANDLED
      ( put_token
      , _
      , p
      , fmt::format
        ( "place '{}', value '{}'"
        , p->place_name()
        , gspc::pnet::type::value::show (p->value())
        )
      );

    UNHANDLED
      ( put_token_response
      , _
      , r
      , fmt::format
        ( "{}"
        , !!r->exception()
          ? gspc::util::exception_printer (r->exception().value()).string()
          : std::string {"success"}
        )
      );

    UNHANDLED (worker_registration_response,_,,"");

    UNHANDLED
      ( workflow_response
      , _
      , r
      , fmt::format
        ( "place '{}', value '{}'"
        , r->place_name()
        , gspc::pnet::type::value::show (r->value())
        )
      );

    UNHANDLED
      ( workflow_response_response
      , _
      , r
      , fmt::format
        ( "content '{}'"
        , gspc::util::visit<std::string>
          ( r->get()
          , [] (workflow_response_response::value_t const& value)
            {
              return fmt::format ("{}", gspc::pnet::type::value::show (value));
            }
          , [] (std::exception_ptr const& error)
            {
              return gspc::util::exception_printer (error).string();
            }
          )
        )
      );

#undef UNHANDLED
  }
