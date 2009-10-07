/*
 * =====================================================================================
 *
 *       Filename:  ConnectionStrategy.hpp
 *
 *    Description:  The gateway between the SEDA stage world and a connection
 *
 *        Version:  1.0
 *        Created:  09/08/2009 04:58:44 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SEDA_COMM_CONNECITONSTRATEGY_HPP
#define SEDA_COMM_CONNECITONSTRATEGY_HPP 1

#include <seda/ForwardStrategy.hpp>
#include <seda/comm/Connection.hpp>
#include <seda/comm/ConnectionListener.hpp>

namespace seda { namespace comm {
  class ConnectionStrategy : public seda::ForwardStrategy, seda::comm::ConnectionListener {
  public:
    ConnectionStrategy(const std::string &targetStage, const seda::comm::Connection::ptr_t &conn)
      : ForwardStrategy(targetStage), conn_(conn)
    {
      conn_->registerListener(this);
    }
    ~ConnectionStrategy() {
      conn_->removeListener(this); 
    }

    void perform(const IEvent::Ptr &toSend);
    void onMessage(const seda::comm::SedaMessage &);

    Connection::ptr_t connection() { return conn_; }

    void onStageStart(const std::string &stageName);
    void onStageStop(const std::string &stageName);
  private:
    Connection::ptr_t conn_;
  };
}}

#endif
