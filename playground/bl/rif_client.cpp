// bernd.loerwald@itwm.fraunhofer.de

#include <playground/bl/rpc/client.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <future>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

int main (int argc, char** argv)
{
  boost::asio::io_service io_service;

  if (argc <= 1)
  {
    throw std::logic_error
      ("usage: " + std::string (argv[0]) + " <binary_to_start> [<arg>]...");
  }

  boost::asio::io_service::work io_service_work_ (io_service);
  const boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
    io_service_thread ([&io_service]() { io_service.run(); });

  remote_endpoint endpoint (io_service, "localhost", 13331);

  remote_function<pid_t (std::string, std::vector<std::string>)> start
    (endpoint, "start");
  sync_remote_function<void (pid_t)> stop
    (endpoint, "stop");

  std::vector<std::string> args;
  for (int i (2); i < argc; ++i)
  {
    args.push_back (argv[i]);
  }
  std::future<pid_t> future_pid (start (argv[1], args));

  std::future_status status;
  do
  {
    status = future_pid.wait_for (std::chrono::milliseconds (500));
    if (status == std::future_status::deferred)
    {
      std::cout << "deferred\n";
    }
    else if (status == std::future_status::timeout)
    {
      std::cout << "timeout\n";
    }
    else if (status == std::future_status::ready)
    {
      std::cout << "ready!\n";
    }
  }
  while (status != std::future_status::ready);

  std::this_thread::sleep_for (std::chrono::seconds (2));

  stop (future_pid.get());

  io_service.stop();

  return 0;
}
