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

#include <sdpa/events/Codec.hpp>

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
        Codec codec;
        seda::comm::SedaMessage::Ptr seda_msg(new seda::comm::SedaMessage(
          sdpa_evt->from()
          , sdpa_evt->to()
          , codec.encode(sdpa_evt)
		  , sdpa_evt->id()
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
        Codec codec;
        seda::IEvent::Ptr evt(codec.decode(seda_msg->payload()));
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
