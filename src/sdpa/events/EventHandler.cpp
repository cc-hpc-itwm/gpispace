// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <sdpa/events/EventHandler.hpp>

#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/events/JobFailedEvent.hpp>
#include <sdpa/events/WorkerRegistrationEvent.hpp>
#include <sdpa/events/put_token.hpp>
#include <sdpa/events/workflow_response.hpp>

#include <we/type/value/show.hpp>

#include <util-generic/functor_visitor.hpp>
#include <util-generic/print_container.hpp>
#include <util-generic/print_exception.hpp>

#include <boost/format.hpp>

#include <stdexcept>
#include <string>

namespace sdpa
{
  namespace events
  {
    namespace detail
    {
      struct semi{};
    }

#define UNHANDLED(Event, Sep, Var, Message...)                        \
    void EventHandler::handle ## Sep ## Event                         \
      (fhg::com::p2p::address_t const&, const Event* Var)             \
    {                                                                 \
      throw std::runtime_error                                        \
        (str (::boost::format ("%1%%2%") % #Event % Message));          \
    } using ::sdpa::events::detail::semi

    UNHANDLED (CancelJobAckEvent,,,"");

    UNHANDLED (CancelJobEvent,,,"");

    UNHANDLED (DeleteJobEvent,,,"");

    UNHANDLED
      ( ErrorEvent
      ,
      , e
      , ( ::boost::format (": error_code '%1%', reason '%2%'")
        % e->error_code()
        % e->reason()
        )
      );

    UNHANDLED (JobFailedAckEvent,,,"");

    UNHANDLED
      ( JobFailedEvent
      ,
      , f
      , ( ::boost::format (": error_message '%1%'")
        % f->error_message()
        )
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
      , ( ::boost::format
            ("name '%1%', host '%2%', capabilites '%3%', shm_size '%4%'")
        % w->name()
        % w->hostname()
        % fhg::util::print_container
            ( "{", ", ", "}", w->capabilities()
            , [] (auto& os, auto const& capability) -> decltype (os)
              {
                return os << capability.name();
              }
            )
        % w->allocated_shared_memory_size()
       )
      );

    UNHANDLED
      ( put_token
      , _
      , p
      , ( ::boost::format ("place '%1%', value '%2%'")
        % p->place_name()
        % ::pnet::type::value::show (p->value())
        )
      );

    UNHANDLED
      ( put_token_response
      , _
      , r
      , ( ::boost::format ("%1%")
        % ( !!r->exception()
          ? fhg::util::exception_printer (r->exception().get()).string()
          : std::string {"success"}
          )
        )
      );

    UNHANDLED (worker_registration_response,_,,"");

    UNHANDLED
      ( workflow_response
      , _
      , r
      , ( ::boost::format ("place '%1%', value '%2%'")
        % r->place_name()
        % ::pnet::type::value::show (r->value())
        )
      );

    UNHANDLED
      ( workflow_response_response
      , _
      , r
      , ( ::boost::format ("content '%1%'")
        % fhg::util::visit<std::string>
          ( r->get()
          , [] (workflow_response_response::value_t const& value)
            {
              return str ( ::boost::format ("%1%")
                         % ::pnet::type::value::show (value)
                         );
            }
          , [] (std::exception_ptr const& error)
            {
              return fhg::util::exception_printer (error).string();
            }
          )
        )
      );

#undef UNHANDLED
  }
}
