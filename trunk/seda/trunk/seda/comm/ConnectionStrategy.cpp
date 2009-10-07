/*
 * =====================================================================================
 *
 *       Filename:  ConnectionStrategy.cpp
 *
 *    Description:  implementation of the seda <-> comm gateway
 *
 *        Version:  1.0
 *        Created:  09/09/2009 10:04:15 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include "ConnectionStrategy.hpp"

using namespace seda::comm;

void ConnectionStrategy::perform(const IEvent::Ptr &toSend)
{
  if (seda::comm::SedaMessage *msg = dynamic_cast<seda::comm::SedaMessage*>(toSend.get()))
  {
    conn_->send(*msg);
  }
  else
  {
    SEDA_LOG_ERROR("could not send event (does not inherit SedaMessage): " << toSend->str());
  }
}

void ConnectionStrategy::onMessage(const seda::comm::SedaMessage &recvMsg)
{
  ForwardStrategy::perform(SedaMessage::Ptr(new SedaMessage(recvMsg)));
}

void ConnectionStrategy::onStageStart(const std::string &stageName)
{
  conn_->start();
}

void ConnectionStrategy::onStageStop(const std::string &stageName)
{
  conn_->stop();
}

