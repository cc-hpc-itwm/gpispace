/*
   Copyright (C) 2009 Alexander Petry <alexander.petry@itwm.fraunhofer.de>.

   This file is part of seda.

   seda is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   seda is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with seda; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

*/

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
      DLOG(DEBUG, "use count = " << conn_.use_count());
      DLOG(DEBUG, "conn = " << conn_.get());
    }

    virtual ~ConnectionStrategy() {
      DLOG(DEBUG, "destructing connection strategy");
      DLOG(DEBUG, "underlying connection..." << connection().get() << " count = " << connection().use_count() );
    }

    void perform(const IEvent::Ptr &toSend);
    void onMessage(const seda::comm::SedaMessage &);

    const Connection::ptr_t &connection() { return conn_; }

    virtual void onStageStart(const std::string & s)
    {
      ForwardStrategy::onStageStart (s);

      DLOG(DEBUG, "starting underlying connection..." << connection().get() << " count = " << connection().use_count() );
      conn_->registerListener(this);
      conn_->start();
      DLOG(DEBUG, "started");
    }
    virtual void onStageStop(const std::string & s)
    {
      DLOG(DEBUG, "stopping underlying connection...");
      conn_->removeListener(this);
      conn_->stop();
      DLOG(DEBUG, "stopped");

      ForwardStrategy::onStageStop (s);
    }
  private:
    seda::comm::Connection::ptr_t conn_;
  };
}}

#endif
