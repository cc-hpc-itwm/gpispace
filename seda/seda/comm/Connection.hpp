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
#include <boost/thread.hpp>
#include <seda/shared_ptr.hpp>
#include <seda/comm/exception.hpp>
#include <seda/comm/ConnectionListener.hpp>
#include <seda/comm/Locator.hpp>
#include <seda/comm/SedaMessage.hpp>
#include <seda/comm/option.hpp>

namespace seda { namespace comm {
    namespace option
    {
      struct enable_compression : public detail::option<bool, enable_compression>
      {
        explicit
        enable_compression (bool b = true)
          : super(b)
        {}
      };
    }

  class Connection {
  public:
    typedef shared_ptr<Connection> ptr_t;

    virtual ~Connection() {}

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void send(const seda::comm::SedaMessage &m) = 0;
    virtual bool recv(seda::comm::SedaMessage &m, const bool block = true) = 0;

    virtual const Locator::ptr_t &locator() const = 0;
    void registerListener(ConnectionListener *);
    void removeListener(ConnectionListener *);

    void set_option (option::enable_compression const &);
    void get_option (option::enable_compression &);
  protected:
    void notifyListener(const seda::comm::SedaMessage &msg);
    bool has_listeners() const;
  private:
    boost::recursive_mutex listener_mtx_;
    typedef std::list<ConnectionListener*> listener_list_t;
    listener_list_t listener_list_;
    bool compression_enabled_;
  };
}}

#endif // SEDA_COMM_CONNECTION_HPP
