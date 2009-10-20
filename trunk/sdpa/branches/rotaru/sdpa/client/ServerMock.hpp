/*
 * =====================================================================================
 *
 *       Filename:  ServerMock.hpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/29/2009 05:05:06 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_CLIENT_SERVER_MOCK_HPP
#define SDPA_CLIENT_SERVER_MOCK_HPP 1

#include <seda/Strategy.hpp>

#include <sdpa/events/SubmitJobEvent.hpp>
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/QueryJobStatusEvent.hpp>
#include <sdpa/events/JobStatusReplyEvent.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/RetrieveJobResultsEvent.hpp>
#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/DeleteJobEvent.hpp>
#include <sdpa/events/DeleteJobAckEvent.hpp>

namespace sdpa { namespace client {
  class ServerMock : public seda::Strategy
  {
  public:
    explicit
    ServerMock(const std::string &replyStage)
      : seda::Strategy(replyStage + ".mock"), reply_(replyStage) {}

    void perform(const seda::IEvent::Ptr &event)
    {
      namespace se = sdpa::events;

      DMLOG(DEBUG, "got event: " << event->str());
      if (dynamic_cast<se::SubmitJobEvent*>(event.get())) {
        DMLOG(DEBUG,"sub");
        seda::StageRegistry::instance().lookup(reply_)->send(
          seda::IEvent::Ptr(new se::SubmitJobAckEvent("orchestrator", name(), job_id_t())));
      } else if (se::QueryJobStatusEvent *q = dynamic_cast<se::QueryJobStatusEvent*>(event.get())) {
        DMLOG(DEBUG,"qstat");
        seda::StageRegistry::instance().lookup(reply_)->send(
          seda::IEvent::Ptr(new se::JobStatusReplyEvent("orchestrator"
                                                       , name()
                                                       , q->job_id()
                                                       , "42")));
      } else if (se::CancelJobEvent *cancel = dynamic_cast<se::CancelJobEvent*>(event.get())) {
        DMLOG(DEBUG,"kill");
        seda::StageRegistry::instance().lookup(reply_)->send(
          seda::IEvent::Ptr(new se::CancelJobAckEvent("orchestrator"
                                                     , "client"
                                                    , cancel->job_id())));
      } else if (se::RetrieveJobResultsEvent *retr = dynamic_cast<se::RetrieveJobResultsEvent*>(event.get())) {
        DMLOG(DEBUG,"get");
        seda::StageRegistry::instance().lookup(reply_)->send(
          seda::IEvent::Ptr(new se::JobResultsReplyEvent("orchestrator"
                                                       , name()
                                                       , retr->job_id()
                                                       , "dummy results")));
      } else if (dynamic_cast<se::DeleteJobEvent*>(event.get())) {
        DMLOG(DEBUG,"del");
        seda::StageRegistry::instance().lookup(reply_)->send(
          seda::IEvent::Ptr(new se::DeleteJobAckEvent("orchestrator"
                                                     , name())));
      } else {
        DMLOG(ERROR, "got some strange event: " << event->str());
      }
    }
  private:
    std::string reply_;
  };
}}

#endif
