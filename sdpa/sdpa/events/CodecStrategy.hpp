#ifndef SDPA_EVENTS_CODEC_STRATEGY_HPP
#define SDPA_EVENTS_CODEC_STRATEGY_HPP 1

#include <seda/StrategyDecorator.hpp>
#include <seda/comm/SedaMessage.hpp>

#include <sdpa/sdpa-config.hpp>
#include <sdpa/memory.hpp>
#include <sdpa/logging.hpp>

#include <sdpa/events/Codec.hpp>

namespace sdpa
{
  namespace events
  {
    class EncodeStrategy : public seda::StrategyDecorator
    {
    public:
      typedef sdpa::shared_ptr<EncodeStrategy> ptr_t;

      EncodeStrategy ( const std::string& a_name
                     , const seda::Strategy::Ptr& decorated
                     )
        : seda::StrategyDecorator (a_name + "-encode", decorated)
      {}

      virtual void perform (const seda::IEvent::Ptr& e)
      {
        if (SDPAEvent * sdpa_evt = dynamic_cast<SDPAEvent*>(e.get()))
        {
          Codec codec;
          seda::comm::SedaMessage::Ptr seda_msg
            (new seda::comm::SedaMessage( sdpa_evt->from()
                                        , sdpa_evt->to()
                                        , codec.encode (sdpa_evt)
                                        //! \todo Explain the 42
                                        , 42
                                        )
            );

          seda::StrategyDecorator::perform (seda_msg);
        }
        else
        {
          LOG ( ERROR
              , "could not encode event (not of type SDPAEvent): " << e->str()
              );

          throw std::runtime_error ("could not encode non-SDPAEvent");
        }
      }
    };

    class DecodeStrategy : public seda::StrategyDecorator
    {
    public:
      typedef sdpa::shared_ptr<DecodeStrategy> ptr_t;

      DecodeStrategy ( const std::string& a_name
                     , const seda::Strategy::Ptr& decorated
                     )
        : seda::StrategyDecorator (a_name + "-decode", decorated)
      {}

      virtual void perform (const seda::IEvent::Ptr &e)
      {
        if ( seda::comm::SedaMessage * seda_msg
           = dynamic_cast<seda::comm::SedaMessage*> (e.get())
           )
        {
          Codec codec;
          seda::IEvent::Ptr evt (codec.decode (seda_msg->payload()));
          seda::StrategyDecorator::perform (evt);
        }
        else
        {
          LOG ( ERROR
              , "could not decode event (not of type SedaMessage): " << e->str()
              );

          throw std::runtime_error("could not decode non-SedaMessage");
        }
      }
    };
  }
}

#endif
