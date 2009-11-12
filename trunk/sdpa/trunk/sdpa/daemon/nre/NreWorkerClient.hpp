/*
 * =====================================================================================
 *
 *       Filename:  NreWorkerClient.hpp
 *
 *    Description:  client side for the nre-worker
 *
 *        Version:  1.0
 *        Created:  11/12/2009 11:56:59 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_DAEMON_NRE_WORKER_CLIENT_HPP
#define SDPA_DAEMON_NRE_WORKER_CLIENT_HPP 1

#if defined(HAVE_CONFIG_H)
#include <sdpa/sdpa-config.hpp>
#endif

#include <fhglog/fhglog.hpp>

#include <sdpa/daemon/nre/messages.hpp>
#include <sdpa/daemon/nre/Codec.hpp>

#include <boost/asio.hpp>

namespace sdpa { namespace nre { namespace worker {
  using boost::asio::ip::udp;

  class NreWorkerClient
  {
  public:
    explicit
    NreWorkerClient(const std::string &nre_worker_location)
      : nre_worker_location_(nre_worker_location)
      , socket_(NULL)
    { }

    const std::string &worker_location() const { return nre_worker_location_; }

    void start() throw (std::exception)
    {
      if (socket_) return;

      DLOG(DEBUG, "starting connection to nre-worker process at: " << worker_location());
      {
        std::string worker_host(worker_location());
        unsigned short worker_port(0);

        std::string::size_type sep_pos(worker_location().find(":"));
        if (sep_pos != std::string::npos)
        {
          worker_host = worker_location().substr(0, sep_pos);
          std::stringstream sstr(worker_location().substr(sep_pos+1));
          sstr >> worker_port;
          if (! sstr)
          {
            throw std::runtime_error("could not parse port-information from location: " + worker_location());
          }
        }
        udp::resolver resolver(io_service_);
        udp::resolver::query query(udp::v4(), worker_host.c_str(), "0");
        nre_worker_endpoint_ = *resolver.resolve(query);
        nre_worker_endpoint_.port(worker_port);

      }
      DLOG(INFO, "connecting to nre-worker process at: " << nre_worker_endpoint_);

      socket_ = new udp::socket(io_service_, udp::endpoint(udp::v4(), 0));

    }

    void stop() throw (std::exception)
    {

    }

    void cancel() throw (std::exception)
    {

    }

    sdpa::wf::Activity execute(const sdpa::wf::Activity &in_activity)
    {
      return in_activity;
    }
  private:
    Message *request(const Message &m)
    {
      assert(socket_);

      std::string encoded_message(codec_.encode(m));
      LOG(DEBUG, "sending " << encoded_message.size() << " bytes of data to " << nre_worker_endpoint_ << ": " << encoded_message);
      // send the request
      socket_->send_to(boost::asio::buffer(encoded_message), nre_worker_endpoint_);

      // wait for the reply
      udp::endpoint sender_endpoint;
      size_t reply_length = socket_->receive_from(boost::asio::buffer(data_, max_length), sender_endpoint);
      std::string data(data_, reply_length);
      LOG(DEBUG, "got " << reply_length << " bytes of data from " << sender_endpoint << ": " << data);
      return codec_.decode(data);
    }

    std::string nre_worker_location_;
    boost::asio::io_service io_service_;
    udp::socket *socket_;
    udp::endpoint nre_worker_endpoint_;
    Codec codec_;

    enum { max_length = ((2<<16) - 1) };
    char data_[max_length];
  };
}}}

#endif
