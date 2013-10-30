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
 *       Filename:  Locator.hpp
 *
 *    Description:  locator service
 *
 *        Version:  1.0
 *        Created:  10/23/2009 02:05:33 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SEDA_COMM_LOCATOR_HPP
#define SEDA_COMM_LOCATOR_HPP 1

#include <boost/unordered_map.hpp>
#include <boost/thread.hpp>
#include <string>
#include <ostream>

#include <fhglog/fhglog.hpp>
#include <seda/shared_ptr.hpp>

namespace seda { namespace comm {
  class Location
  {
  public:
    typedef unsigned short port_t;
    typedef std::string host_t;

    Location()
      : name_()
      , host_()
      , port_(0)
    {}

    Location(const std::string &a_name
           , const host_t &a_host
           , const port_t &a_port)
      : name_(a_name)
      , host_(a_host)
      , port_(a_port)
    {}

    Location(const std::string &a_name
           , const std::string &host_port)
      : name_(a_name)
    {
      std::string::size_type colon_pos(host_port.find(':'));
      if (colon_pos != std::string::npos)
      {
        host_ = host_port.substr(0, colon_pos);
        const std::string port_s(host_port.substr(colon_pos+1));

        std::stringstream sstr(port_s);
        sstr >> port_;

        if (! sstr)
          throw std::runtime_error("invalid port: " + port_s);
      }
      else
      {
        throw std::runtime_error("splitting of " + host_port + " into host and port failed!");
      }
    }

    const std::string &name() const { return name_; }
    std::string &name() { return name_; }

    const host_t &host() const { return host_; }
    host_t &host() { return host_; }

    const port_t &port() const { return port_; }
    port_t &port() { return port_; }

  private:
    std::string name_;
    host_t host_;
    port_t port_;
  };

  inline std::ostream &operator<<(std::ostream &os, const Location &loc)
  {
    os << loc.name() << ":" << loc.host() << ":" << loc.port();
    return os;
  }

  class Locator
  {
  public:
    typedef shared_ptr<Locator> ptr_t;
    typedef Location location_t;
    typedef boost::unordered_map<std::string, location_t> location_map_t;
    typedef location_map_t::iterator iterator;
    typedef location_map_t::const_iterator const_iterator;

    const location_t &lookup(const std::string &name) const throw (std::exception)
    {
      boost::unique_lock<boost::recursive_mutex> mtx_lock(mtx_);
      DLOG(DEBUG, "looking up " << name);
      location_map_t::const_iterator loc_it(locations_.find(name));
      if (loc_it != locations_.end())
      {
        const location_t &loc(loc_it->second);
        DLOG(DEBUG, "location of " << loc.name() << " is " << loc.host() << ":" << loc.port());
        return loc;
      }
      else
      {
        DLOG(DEBUG, "not found");
        throw std::runtime_error("could not locate " + name);
      }
    }

    void insert(const std::string &name, const std::string &val)
    {
      insert(name, location_t(name, val));
    }

    void insert(const std::string &name, const location_t::host_t &h, location_t::port_t p)
    {
      insert(name, location_t(name, h, p));
    }

    void insert(const std::string &name, const location_t &location)
    {
      boost::unique_lock<boost::recursive_mutex> mtx_lock(mtx_);
      DLOG(DEBUG, "updating location of " << name << " to " << location.host() << ":" << location.port());
      locations_[name] = location;
    }

    void remove(const std::string &name)
    {
      boost::unique_lock<boost::recursive_mutex> mtx_lock(mtx_);
      locations_.erase(name);
    }

    // boost thread Lockable concept
    void lock()
    {
      mtx_.lock();
    }
    bool try_lock()
    {
      return mtx_.try_lock();
    }
    void unlock()
    {
      mtx_.unlock();
    }

    // make sure you are the owner!
    const_iterator begin() const { return locations_.begin(); }
    const_iterator end() const { return locations_.end(); }
  private:
    location_map_t locations_;
    mutable boost::recursive_mutex mtx_;
  };
}}

#endif
