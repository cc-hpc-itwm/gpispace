/*
 * =====================================================================================
 *
 *       Filename:  test_codec.cpp
 *
 *    Description:  tests the encoding/decoding of events
 *
 *        Version:  1.0
 *        Created:  10/30/2009 01:43:03 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <iostream>

#include <fhglog/fhglog.hpp>
#include <fhglog/Configuration.hpp>
#include <seda/DiscardStrategy.hpp>
#include <seda/AccumulateStrategy.hpp>
#include <sdpa/events/Codec.hpp>
#include <sdpa/events/CodecStrategy.hpp>

int main(int, char **)
{
  using namespace sdpa::events;
  fhg::log::Configurator::configure();  
  
  int errcount(0);

  {
    LOG(DEBUG, "setting up the strategies...");
    seda::DiscardStrategy::Ptr devnull(new seda::DiscardStrategy("devnull"));
    seda::AccumulateStrategy::Ptr acc(new seda::AccumulateStrategy(devnull));
    EncodeStrategy::ptr_t encode(new EncodeStrategy("to-net", acc));

    std::clog << "testing SubmitJobEvent...";
    SubmitJobEvent::Ptr e(new SubmitJobEvent("foo", "bar", "job-id-1", "<desc></desc>", "parent-job-id-0"));
    encode->perform(e);

    if (seda::comm::SedaMessage *msg = dynamic_cast<seda::comm::SedaMessage*>(acc->begin()->get()))
    {
      if (msg->from() != e->from()
       || msg->to() != e->to())
      {
        ++errcount;
        std::clog << "FAILED!" << std::endl;
        std::clog << "\tto or from do not match!" << std::endl;
      }
      else
      {
        std::clog << "OK!" << std::endl;
      }
    }
    else
    {
      ++errcount;
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tencoded event is not of type SedaMessage!" << std::endl;
      std::clog << "\tgot: " << (typeid(acc->begin()->get()).name()) << std::endl;
    }
  }

  {
    LOG(DEBUG, "setting up the strategies...");
    seda::DiscardStrategy::Ptr devnull(new seda::DiscardStrategy("devnull"));
    seda::AccumulateStrategy::Ptr acc(new seda::AccumulateStrategy(devnull));
    DecodeStrategy::ptr_t decode(new DecodeStrategy("from-net", acc));
    EncodeStrategy::ptr_t encode(new EncodeStrategy("to-net", decode));

    std::clog << "testing SubmitJobEvent...";
    SubmitJobEvent::Ptr e(new SubmitJobEvent("foo", "bar", "job-id-1", "<desc></desc>", "parent-job-id-0"));
    encode->perform(e);

    if (SubmitJobEvent *msg = dynamic_cast<SubmitJobEvent*>(acc->begin()->get()))
    {
      if (msg->from() != e->from()
       || msg->to() != e->to()
       || msg->description() != e->description()
      )
      {
        ++errcount;
        std::clog << "FAILED!" << std::endl;
        std::clog << "\tto or from do not match!" << std::endl;
      }
      else
      {
        std::clog << "OK!" << std::endl;
      }
    }
    else
    {
      ++errcount;
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tdecoded event is not of type SubmitJobEvent!" << std::endl;
      std::clog << "\tgot: " << (typeid(acc->begin()->get()).name()) << std::endl;
    }
  }

  return errcount;
}
