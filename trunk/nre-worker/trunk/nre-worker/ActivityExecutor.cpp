/*
 * =====================================================================================
 *
 *       Filename:  UDPActivityExecutor.cpp
 *
 *    Description:  implementation
 *
 *        Version:  1.0
 *        Created:  11/09/2009 04:47:56 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <sstream>
#include <stdexcept>

#include <boost/asio.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <sdpa/daemon/nre/messages.hpp>
#include <sdpa/daemon/nre/Serialization.hpp>

#include "ActivityExecutor.hpp"

using boost::asio::ip::udp;

template <class Archive>
void init_archive(Archive & ar)
{
  ar.register_type(static_cast<sdpa::nre::worker::PingRequest*>(NULL));
  ar.register_type(static_cast<sdpa::nre::worker::PingReply*>(NULL));

  ar.register_type(static_cast<sdpa::nre::worker::ExecuteRequest*>(NULL));
  ar.register_type(static_cast<sdpa::nre::worker::ExecuteReply*>(NULL));
}

namespace sdpa { namespace nre { namespace worker {
  enum { max_length = ((2<<16) - 1) };

  void
  ActivityExecutor::loop()
  {
    LOG(DEBUG, "opening connection on: " << location());

    std::string host(location());
    unsigned short port(0);

    std::string::size_type sep_pos(location().find(":"));
    if (sep_pos != std::string::npos)
    {
      host = location().substr(0, sep_pos);
      std::stringstream sstr(location().substr(sep_pos+1));
      sstr >> port;
      if (! sstr)
      {
        throw std::runtime_error("could not parse port-information from location: " + location());
      }
    }

    udp::endpoint my_endpoint(boost::asio::ip::address::from_string(host), port);

    boost::asio::io_service io_service;
    udp::socket socket(io_service, my_endpoint);

    udp::endpoint real_endpoint = socket.local_endpoint();
    LOG(INFO, "listening on " << real_endpoint);

    for (;;)
    {
      char data[max_length];
      udp::endpoint sender_endpoint;

      size_t length = socket.receive_from(boost::asio::buffer(data, max_length), sender_endpoint);

      {
        data[length] = 0;
        std::string msg(data);
        DLOG(DEBUG, sender_endpoint << " sent me " << length << " bytes of data: " << data);

        // special commands in debug build
#ifndef NDEBUG
        DLOG(DEBUG, "accepting the following \"special\" commands: " << "QUIT");
        if (msg == "QUIT")
        {
          DLOG(INFO, "got QUIT request, returning from loop...");
          return;
        }
#endif

        Request *rqst = NULL;
        try
        {
          std::stringstream sstr(msg);
          boost::archive::text_iarchive ar(sstr);
          init_archive(ar);
          ar >> rqst;
          Reply *rply = rqst->execute(this);
          delete rqst; rqst = NULL;

          if (rply)
          {
            std::ostringstream sstr;
            boost::archive::text_oarchive ar(sstr);
            init_archive(ar);

            ar << rply;

            socket.send_to(boost::asio::buffer(sstr.str()), sender_endpoint);

            delete rply; rply = NULL;
          }
          else
          {
            LOG(DEBUG, "nothing to reply, assuming shutdown...");
            break;
          }
        }
        catch (const std::exception &ex)
        {
          LOG(ERROR, "could not execute the desired request: " << ex.what());
        }
      }
    }
  }
}}}
