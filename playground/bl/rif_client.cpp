// bernd.loerwald@itwm.fraunhofer.de

#include <rpc/client.hpp>

#include <fhg/util/print_exception.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <future>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

int main (int argc, char** argv)
try
{
  if (argc < 4)
  {
    throw std::logic_error
      ( "usage: "
      + std::string (argv[0]) + " <host> <port> <binary_to_start> [<arg>]..."
      );
  }

  boost::asio::io_service io_service;
  boost::asio::io_service::work io_service_work_ (io_service);
  const boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
    io_service_thread ([&io_service]() { io_service.run(); });

  fhg::rpc::remote_endpoint endpoint
    ( io_service
    , argv[1], std::stoi (argv[2])
    , fhg::util::serialization::exception::serialization_functions()
    );

  fhg::rpc::remote_function<pid_t (std::string, std::vector<std::string>)> start
    (endpoint, "start");
  fhg::rpc::sync_remote_function<void (pid_t)> stop
    (endpoint, "stop");

  std::vector<std::string> args;
  for (int i (4); i < argc; ++i)
  {
    args.push_back (argv[i]);
  }
  std::future<pid_t> future_pid (start (argv[3], args));

  future_pid.wait();

  std::this_thread::sleep_for (std::chrono::seconds (1));

  stop (future_pid.get());

  io_service.stop();

  return 0;
}
catch (...)
{
  fhg::util::print_current_exception (std::cerr, "EX: ");
  return 1;
}
