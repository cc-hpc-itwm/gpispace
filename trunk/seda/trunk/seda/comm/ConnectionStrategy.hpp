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

#ifndef SEDA_COMM_CONNECTION_STRATEGY_HPP
#define SEDA_COMM_CONNECTION_STRATEGY_HPP 1

#include <seda/ForwardStrategy.hpp>
#include <seda/comm/Connection.hpp>
#include <seda/comm/ConnectionListener.hpp>

namespace seda { namespace comm {
  class ConnectionStrategy : public seda::ForwardStrategy, public seda::comm::ConnectionListener {
  public:
    typedef shared_ptr<ConnectionStrategy> ptr_t;

    ConnectionStrategy(const std::string &targetStage, const Connection::ptr_t &a_conn)
      : ForwardStrategy(targetStage)
      , ConnectionListener()
      , conn_(a_conn)
    {
      LOG(DEBUG, "use count = " << conn_.use_count());
      LOG(DEBUG, "conn = " << conn_.get());
    }

    virtual ~ConnectionStrategy() {
      conn_.reset();
      DLOG(DEBUG, "destructing connection strategy");
      LOG(DEBUG, "underlying connection..." << connection().get() << " count = " << connection().use_count() );
    }

    void perform(const IEvent::Ptr &toSend);
    void onMessage(const seda::comm::SedaMessage &);

    const Connection::ptr_t &connection() { return conn_; }

    virtual void onStageStart(const std::string &stageName)
    {
      LOG(DEBUG, "starting underlying connection..." << connection().get() << " count = " << connection().use_count() );
      connection()->registerListener(this);
      connection()->start();
    }
    virtual void onStageStop(const std::string &stageName)
    {
      DLOG(DEBUG, "stopping underlying connection...");
      connection()->removeListener(this);
      connection()->stop();
      DLOG(DEBUG, "stopped");
    }
  private:
    seda::comm::Connection::ptr_t conn_;
  };
}}

#endif
