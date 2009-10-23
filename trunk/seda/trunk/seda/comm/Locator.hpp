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

#include <fhglog/fhglog.hpp>
#include <seda/shared_ptr.hpp>
#include <string>
#include <map>

namespace seda { namespace comm {
  class Locator
  {
  public:
    typedef shared_ptr<Locator> ptr_t;
    typedef unsigned short port_t;
    typedef std::string host_t;
    typedef std::pair<host_t, port_t> location_t;
    typedef std::map<std::string, location_t> location_map_t;
    
    const location_t &lookup(const std::string &name) const throw (std::exception)
    {
      DLOG(DEBUG, "looking up " << name);
      location_map_t::const_iterator loc_it(locations_.find(name));
      if (loc_it != locations_.end())
      {
        const location_t &loc(loc_it->second);
        DLOG(DEBUG, "location of " << name << " is " << loc.first << ":" << loc.second);
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
      insert(name, split_host_port(val));
    }
    void insert(const std::string &name, const host_t &h, port_t p)
    {
      insert(name, std::make_pair(h,p));
    }
    void insert(const std::string &name, const location_t &location)
    {
      DLOG(DEBUG, "updating location of " << name << " to " << location.first << ":" << location.second);
      locations_[name] = location;
    }

    void remove(const std::string &name)
    {
      locations_.erase(name);
    }
  private:
    location_map_t locations_;    

    location_t split_host_port(const std::string &val) const
    {
      std::string::size_type colon_pos(val.find(':'));
      if (colon_pos != std::string::npos)
      {
        const host_t host_s(val.substr(0, colon_pos));
        const std::string port_s(val.substr(colon_pos+1));
        port_t port;

        std::stringstream sstr(port_s);
        sstr >> port;

        if (! sstr)
          throw std::runtime_error("invalid port: " + port_s);
        else
          return std::make_pair(host_s, port);
      }
      throw std::runtime_error("splitting of " + val + " into host and port failed");
    }
  };
}}

#endif
