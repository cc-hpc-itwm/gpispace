#ifndef SEDA_COMM_ZMQ_CONNECTION_HPP
#define SEDA_COMM_ZMQ_CONNECTION_HPP 1

#include <map>
#include <deque>
#include <string>
#include <seda/common.hpp>
#include <seda/comm/Encodeable.hpp>
#include <seda/comm/Connection.hpp>
#include <seda/comm/SedaMessage.hpp>
#include <boost/thread.hpp>

#include <zmq.hpp>

namespace seda {
namespace comm {
  class ZMQConnection : public seda::comm::Connection {
  public:
    typedef int exchange_t;
    typedef int queue_t;
    typedef std::map<address_type, exchange_t> address_map_t;

    ZMQConnection(const std::string &locator
                , const std::string &name
                , const std::string &in_interface
                , const std::string &out_interface);
    ~ZMQConnection();

    void start();
    void stop();

    void send(const seda::comm::SedaMessage &);
    bool recv(seda::comm::SedaMessage &m, const bool block = true) throw(boost::thread_interrupted);

    void operator()();
  protected:
    exchange_t locate(const address_type &);
  private:
    // no copy constructor
    ZMQConnection(const ZMQConnection&);

    SEDA_DECLARE_LOGGER();

    // zmq related variables
    std::string locator_host_;
    std::string name_;
    std::string in_iface_;
    std::string out_iface_;

    zmq::dispatcher_t *dispatcher_;
    zmq::locator_t *locator_;
    zmq::i_thread *io_thread_;
    zmq::api_thread_t *send_api_;
    zmq::api_thread_t *recv_api_;

    queue_t incoming_queue_;
    exchange_t self_exchange_;
    address_map_t exchanges_;

    // asynchronous receive implementation
    boost::recursive_mutex mtx_;
    boost::condition_variable_any recv_cond_;
    boost::thread *recv_thread_;

    std::deque<seda::comm::SedaMessage> incoming_messages_;
    std::size_t recv_waiting_;
  };
}
}

#endif
