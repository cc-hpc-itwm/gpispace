/*
 * =====================================================================================
 *
 *       Filename:  Connection.hpp
 *
 *    Description:  defines the interface to a generic connection
 *
 *        Version:  1.0
 *        Created:  09/08/2009 02:34:26 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SEDA_COMM_CONNECTION_HPP
#define SEDA_COMM_CONNECTION_HPP

#include <list>
#include <seda/shared_ptr.hpp>
#include <seda/comm/ConnectionListener.hpp>
#include <seda/comm/SedaMessage.hpp>

namespace seda { namespace comm {
  class Connection {
  public:
    typedef seda::shared_ptr<Connection> ptr_t;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void send(const seda::comm::SedaMessage &m) = 0;
    virtual bool recv(seda::comm::SedaMessage &m, const bool block = true) = 0;

    void registerListener(ConnectionListener *);
    void removeListener(ConnectionListener *);
  protected:
    void notifyListener(const seda::comm::SedaMessage &msg) const;
    typedef std::list<ConnectionListener*> listener_list_t;
    listener_list_t listener_list_;
  };
}}

#endif // SEDA_COMM_CONNECTION_HPP
