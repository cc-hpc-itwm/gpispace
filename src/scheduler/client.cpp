// Copyright (C) 2010-2011,2013-2016,2018-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/scheduler/client.hpp>

#include <gspc/scheduler/events/CancelJobAckEvent.hpp>
#include <gspc/scheduler/events/CancelJobEvent.hpp>
#include <gspc/scheduler/events/Codec.hpp>
#include <gspc/scheduler/events/DeleteJobAckEvent.hpp>
#include <gspc/scheduler/events/DeleteJobEvent.hpp>
#include <gspc/scheduler/events/ErrorEvent.hpp>
#include <gspc/scheduler/events/JobFailedEvent.hpp>
#include <gspc/scheduler/events/JobFinishedEvent.hpp>
#include <gspc/scheduler/events/SubmitJobAckEvent.hpp>
#include <gspc/scheduler/events/SubmitJobEvent.hpp>
#include <gspc/scheduler/events/SubscribeAckEvent.hpp>
#include <gspc/scheduler/events/SubscribeEvent.hpp>
#include <gspc/scheduler/events/put_token.hpp>
#include <gspc/scheduler/events/workflow_response.hpp>

#include <boost/lexical_cast.hpp>
#include <optional>
#include <boost/system/error_code.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <functional>
#include <stdexcept>
#include <string>
#include <utility>


  namespace gspc::scheduler::client
  {
    Client::Client ( gspc::com::host_t const& top_level_agent_host
                   , gspc::com::port_t const& top_level_agent_port
                   , std::unique_ptr<::boost::asio::io_service> peer_io_service
                   , gspc::Certificates const& certificates
                   )
      : m_peer ( std::move (peer_io_service)
               , gspc::com::port_t (0)
               , certificates
               , top_level_agent_host
               , top_level_agent_port
               )
    {
      m_peer.async_recv ( std::bind ( &Client::handle_recv
                                    , this
                                    , std::placeholders::_1
                                    )
                        );
    }

    Client::~Client()
    {
      _stopping = true;
    }

    void Client::handle_recv (gspc::com::peer_t::Received received)
    {
      static gspc::scheduler::events::Codec const codec {};

      if (! received.ec())
      {
        auto const& message {received.message()};
        gspc::scheduler::events::SchedulerEvent::Ptr const evt
          (codec.decode (std::string (message.data.begin(), message.data.end())));
        m_incoming_events.put (evt);
      }
      else if ( received.ec() == ::boost::system::errc::operation_canceled
              || received.ec() == ::boost::system::errc::network_down
              || received.ec() == ::boost::asio::error::eof
              || received.ec() == ::boost::asio::error::connection_reset
              )
      {
        _stopping = true;
      }
      else if (!_stopping)
      {
        auto const& message {received.message()};
        if (message.header.src != m_peer.address())
        {
          gspc::scheduler::events::ErrorEvent::Ptr const
            error (new gspc::scheduler::events::ErrorEvent ( gspc::scheduler::events::ErrorEvent::SCHEDULER_EUNKNOWN
                                               , "receiving response failed: " + ::boost::lexical_cast<std::string>(received.ec())
                                               )
                 );
          m_incoming_events.put (error);
        }
      }

      if (!_stopping)
      {
        m_peer.async_recv ( std::bind ( &Client::handle_recv
                                      , this
                                      , std::placeholders::_1
                                      )
                          );
      }
    }

    namespace
    {
      [[noreturn]] void handle_error_and_unexpected_event
        (gspc::scheduler::events::SchedulerEvent::Ptr reply)
      {
        if ( auto* const err
           = dynamic_cast<gspc::scheduler::events::ErrorEvent*> (reply.get())
           )
        {
          throw std::runtime_error
            ( "Error: reason := "
            + err->reason()
            + " code := "
            + std::to_string (err->error_code())
            );
        }

        throw std::runtime_error
          ("Unexpected reply: " + std::string (typeid (reply.get()).name()));
      }
    }

    template<typename Expected, typename Sent>
      Expected Client::send_and_wait_for_reply (Sent event)
    {
      std::lock_guard<std::mutex> const _ (_make_client_thread_safe);

      static gspc::scheduler::events::Codec const codec {};
      m_peer.send (codec.encode (&event));

      const gspc::scheduler::events::SchedulerEvent::Ptr reply (m_incoming_events.get());
      if (auto* const e = dynamic_cast<Expected*> (reply.get()))
      {
        return std::move (*e);
      }

      handle_error_and_unexpected_event (reply);
    }

    gspc::scheduler::status::code Client::wait_for_terminal_state
      (job_id_t id, job_info_t& job_info)
    {
      send_and_wait_for_reply<gspc::scheduler::events::SubscribeAckEvent>
        (gspc::scheduler::events::SubscribeEvent (id));
     gspc::scheduler::events::SchedulerEvent::Ptr const reply (m_incoming_events.get());

      if ( auto* const job_finished
         = dynamic_cast<gspc::scheduler::events::JobFinishedEvent*> (reply.get())
         )
      {
        if (job_finished->job_id() != id)
        {
          throw std::runtime_error ("got status change for different job");
        }

        _job_results.emplace (id, job_finished->result());
        return gspc::scheduler::status::FINISHED;
      }
      else if ( auto* const job_failed
              = dynamic_cast<gspc::scheduler::events::JobFailedEvent*> (reply.get())
              )
      {
        if (job_failed->job_id() != id)
        {
          throw std::runtime_error ("got status change for different job");
        }
        job_info.error_message = job_failed->error_message();
        return gspc::scheduler::status::FAILED;
      }
      else if ( auto* const cancel_ack
              = dynamic_cast<gspc::scheduler::events::CancelJobAckEvent*> (reply.get())
              )
      {
        if (cancel_ack->job_id() != id)
        {
          throw std::runtime_error ("got status change for different job");
        }
        return gspc::scheduler::status::CANCELED;
      }

      handle_error_and_unexpected_event (reply);
    }

    gspc::scheduler::job_id_t Client::submitJob (gspc::we::type::Activity activity)
    {
      return send_and_wait_for_reply<gspc::scheduler::events::SubmitJobAckEvent>
        (gspc::scheduler::events::SubmitJobEvent (std::nullopt, std::move (activity), std::nullopt)).job_id();
    }

    void Client::cancelJob (job_id_t const& jid)
    {
      send_and_wait_for_reply<gspc::scheduler::events::CancelJobAckEvent>
        (gspc::scheduler::events::CancelJobEvent (jid));
    }

    void Client::put_token
      (job_id_t job_id, std::string place_name, gspc::pnet::type::value::value_type value)
    {
      std::string const put_token_id
        (::boost::uuids::to_string (::boost::uuids::random_generator()()));

      gspc::scheduler::events::put_token_response const response
        ( send_and_wait_for_reply<gspc::scheduler::events::put_token_response>
            ( gspc::scheduler::events::put_token ( job_id
                                      , put_token_id
                                      , place_name
                                      , value
                                      )
            )
        );

      if (response.put_token_id() != put_token_id)
      {
        throw std::logic_error ("received put_token_response for different put_token");
      }

      return response.get();
    }

    gspc::pnet::type::value::value_type Client::workflow_response
      (job_id_t job_id, std::string place_name, gspc::pnet::type::value::value_type value)
    {
      std::string const workflow_response_id
        (::boost::uuids::to_string (::boost::uuids::random_generator()()));

      gspc::scheduler::events::workflow_response_response const response
        ( send_and_wait_for_reply<gspc::scheduler::events::workflow_response_response>
            ( gspc::scheduler::events::workflow_response ( job_id
                                              , workflow_response_id
                                              , place_name
                                              , value
                                              )
            )
        );

      if (response.workflow_response_id() != workflow_response_id)
      {
        throw std::logic_error
          ("received workflow_response_response for different workflow_response");
      }

      return response.get();
    }

    void Client::deleteJob (job_id_t const& jid)
    {
      send_and_wait_for_reply<gspc::scheduler::events::DeleteJobAckEvent>
        (gspc::scheduler::events::DeleteJobEvent (jid));

      _job_results.erase (jid);
    }

    gspc::we::type::Activity Client::result (gspc::scheduler::job_id_t const& job) const
    {
      if (!_job_results.count (job))
      {
        throw std::runtime_error ("couldn't find any resulted stored for the job " + job);
      }

      return _job_results.at (job);
    }
  }
