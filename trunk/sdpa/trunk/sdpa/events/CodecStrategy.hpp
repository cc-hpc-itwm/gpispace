/*
 * =====================================================================================
 *
 *       Filename:  CodecStrategy.hpp
 *
 *    Description:  encodes / decodes events
 *
 *        Version:  1.0
 *        Created:  10/28/2009 01:20:37 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_EVENTS_CODEC_STRATEGY_HPP
#define SDPA_EVENTS_CODEC_STRATEGY_HPP 1

#include <seda/StrategyDecorator.hpp>
#include <seda/comm/SedaMessage.hpp>

#include <sdpa/sdpa-config.hpp>
#include <sdpa/memory.hpp>
#include <sdpa/logging.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sdpa/events/Serialization.hpp>

namespace sdpa { namespace events {
  class EncodeStrategy : public seda::StrategyDecorator
  {
  public:
    typedef sdpa::shared_ptr<EncodeStrategy> ptr_t;

    EncodeStrategy(const std::string &a_name
                ,  const seda::Strategy::Ptr &decorated)
      : seda::StrategyDecorator(a_name + "-encode", decorated)
    {}

    virtual ~EncodeStrategy() {}

    virtual void perform(const seda::IEvent::Ptr &e)
    {
      if (SDPAEvent * sdpa_evt = dynamic_cast<SDPAEvent*>(e.get()))
      {
        std::ostringstream sstr;
        boost::archive::text_oarchive ar(sstr);

        ar.register_type(static_cast<CancelJobAckEvent*>(NULL));
        ar.register_type(static_cast<CancelJobEvent*>(NULL));
        ar.register_type(static_cast<ConfigReplyEvent*>(NULL));
        ar.register_type(static_cast<ConfigRequestEvent*>(NULL));
        ar.register_type(static_cast<DeleteJobAckEvent*>(NULL));
        ar.register_type(static_cast<DeleteJobEvent*>(NULL));
        ar.register_type(static_cast<ErrorEvent*>(NULL));
        ar.register_type(static_cast<JobFailedAckEvent*>(NULL));
        ar.register_type(static_cast<JobFailedEvent*>(NULL));
        ar.register_type(static_cast<JobFinishedAckEvent*>(NULL));
        ar.register_type(static_cast<JobFinishedEvent*>(NULL));
        ar.register_type(static_cast<JobResultsReplyEvent*>(NULL));
        ar.register_type(static_cast<JobStatusReplyEvent*>(NULL));
        ar.register_type(static_cast<LifeSignEvent*>(NULL));
        ar.register_type(static_cast<QueryJobStatusEvent*>(NULL));
        ar.register_type(static_cast<RequestJobEvent*>(NULL));
        ar.register_type(static_cast<RetrieveJobResultsEvent*>(NULL));
        ar.register_type(static_cast<SubmitJobAckEvent*>(NULL));
        ar.register_type(static_cast<SubmitJobEvent*>(NULL));
        ar.register_type(static_cast<WorkerRegistrationAckEvent*>(NULL));
        ar.register_type(static_cast<WorkerRegistrationEvent*>(NULL));

        ar << sdpa_evt;
        seda::comm::SedaMessage::Ptr seda_msg(new seda::comm::SedaMessage(
          sdpa_evt->from()
          , sdpa_evt->to()
          , sstr.str()
        ));

        seda::StrategyDecorator::perform(seda_msg);
      }
      else
      {
        LOG(ERROR, "could not encode event (not of type SDPAEvent): " << e->str());
        throw std::runtime_error("could not encode non-SDPAEvent");
      }
    }
  };

  class DecodeStrategy : public seda::StrategyDecorator
  {
  public:
    typedef sdpa::shared_ptr<DecodeStrategy> ptr_t;

    DecodeStrategy(const std::string &a_name
                ,  const seda::Strategy::Ptr &decorated)
      : seda::StrategyDecorator(a_name + "-decode", decorated)
    {}

    virtual ~DecodeStrategy() {}

    virtual void perform(const seda::IEvent::Ptr &e)
    {
      if (seda::comm::SedaMessage * seda_msg = dynamic_cast<seda::comm::SedaMessage*>(e.get()))
      {
        std::istringstream sstr(seda_msg->payload());
        boost::archive::text_iarchive ar(sstr);

        ar.register_type(static_cast<CancelJobAckEvent*>(NULL));
        ar.register_type(static_cast<CancelJobEvent*>(NULL));
        ar.register_type(static_cast<ConfigReplyEvent*>(NULL));
        ar.register_type(static_cast<ConfigRequestEvent*>(NULL));
        ar.register_type(static_cast<DeleteJobAckEvent*>(NULL));
        ar.register_type(static_cast<DeleteJobEvent*>(NULL));
        ar.register_type(static_cast<ErrorEvent*>(NULL));
        ar.register_type(static_cast<JobFailedAckEvent*>(NULL));
        ar.register_type(static_cast<JobFailedEvent*>(NULL));
        ar.register_type(static_cast<JobFinishedAckEvent*>(NULL));
        ar.register_type(static_cast<JobFinishedEvent*>(NULL));
        ar.register_type(static_cast<JobResultsReplyEvent*>(NULL));
        ar.register_type(static_cast<JobStatusReplyEvent*>(NULL));
        ar.register_type(static_cast<LifeSignEvent*>(NULL));
        ar.register_type(static_cast<QueryJobStatusEvent*>(NULL));
        ar.register_type(static_cast<RequestJobEvent*>(NULL));
        ar.register_type(static_cast<RetrieveJobResultsEvent*>(NULL));
        ar.register_type(static_cast<SubmitJobAckEvent*>(NULL));
        ar.register_type(static_cast<SubmitJobEvent*>(NULL));
        ar.register_type(static_cast<WorkerRegistrationAckEvent*>(NULL));
        ar.register_type(static_cast<WorkerRegistrationEvent*>(NULL));

        SDPAEvent *sdpa_evt(NULL);
        ar >> sdpa_evt;

        seda::IEvent::Ptr evt(sdpa_evt);
        seda::StrategyDecorator::perform(evt);
      }
      else
      {
        LOG(ERROR, "could not decode event (not of type SedaMessage): " << e->str());
        throw std::runtime_error("could not decode non-SedaMessage");
      }
    }
  };
}}

#endif
