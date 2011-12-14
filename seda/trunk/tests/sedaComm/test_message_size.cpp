#include <iostream>
#include <algorithm>

#include <seda/comm/comm.hpp>
#include <seda/comm/Connection.hpp>
#include <seda/comm/UDPConnection.hpp>
#include <seda/comm/ConnectionFactory.hpp>
#include <seda/comm/ConnectionStrategy.hpp>

struct process
{
  process (std::string const & url, std::string const & name)
    : url_ (url)
    , name_(name)
  {
    LOG(INFO, "creating process " << name << " at " << url);

    seda::comm::ConnectionFactory factory;
    seda::comm::ConnectionParameters params("udp", url_, name_);
    conn_ = factory.createConnection(params);
  }

  void start ()
  {
    conn_->start();
  }

  seda::comm::Connection::ptr_t connection ()
  {
    return conn_;
  }

  std::string const & name () const { return name_; }
  std::string const & url () const { return url_; }

private:
  std::string url_;
  std::string name_;
  seda::comm::Connection::ptr_t conn_;
};


int main (int ac, char **av)
{
  fhg::log::Configurator::configure();
  seda::comm::initialize(ac, av);

  process p1 ("127.0.0.1:5000", "A");
  process p2 ("127.0.0.1:5001", "B");

  p1.connection()->locator()->insert(p2.name(), p2.url());
  p2.connection()->locator()->insert(p1.name(), p1.url());

  p1.start();
  p2.start();

  std::size_t cnt (0);
  std::size_t good (1);
  std::size_t bad (0);
  std::size_t curr (good);

  while (cnt < 100 && (bad != curr))
  {
    // sending messages in increasing number of bytes
    std::string data;
    std::fill_n ( std::back_inserter (data)
		, curr
		, 'A'
		);
    try
    {
      seda::comm::SedaMessage msg_out(p1.name(), p2.name(), data, cnt++);
      p1.connection()->send (msg_out);

      seda::comm::SedaMessage msg_in;
      p2.connection()->recv (msg_in);

      good = curr;
      if (bad > good)
      {
	curr += (bad - good) / 2;
      }
      else
      {
	curr *= 2;
      }
    }
    catch (std::exception const & ex)
    {
      LOG(ERROR, "sending message of size " << curr << " failed: " << ex.what());
      bad = curr;
      curr -= (bad - good) / 2;
    }
  }

  LOG(INFO, "maximum possible payload size: " << good << " found after " << cnt << " iterations.");

  return EXIT_SUCCESS;
}
