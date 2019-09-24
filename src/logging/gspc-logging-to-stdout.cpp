#include <logging/endpoint.hpp>
#include <logging/stream_receiver.hpp>

#include <fhg/util/boost/program_options/generic.hpp>
#include <util-generic/ostream/put_time.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/syscall/process_signal_block.hpp>
#include <util-generic/syscall/signal_set.hpp>

#include <cstdlib>
#include <iostream>
#include <vector>

int main (int argc, char** argv)
try
{
  fhg::util::syscall::signal_set const signals {SIGINT, SIGTERM};
  fhg::util::syscall::process_signal_block const signal_block (signals);

  namespace po = fhg::util::boost::program_options;
  po::option<std::vector<fhg::logging::endpoint>> const emitters
    {"emitters", "list of emitters"};

  boost::program_options::variables_map const vm
    ( po::options ("GPI-Space Logging to stdout")
    . require (emitters)
    . store_and_notify (argc, argv)
    );

  fhg::logging::stream_receiver receiver
    ( emitters.get_from (vm)
    , [] (fhg::logging::message const& message)
      {
        std::cout << message._process_id << "." << message._thread_id
                  << "@" << message._hostname << " "
                  << fhg::util::ostream::put_time<std::chrono::system_clock>
                       (message._timestamp)
                  << " " << message._category
                  << " " << message._content << "\n";
      }
    );

  fhg::util::syscall::sigwaitinfo (&signals._, nullptr);

  return EXIT_SUCCESS;
}
catch (...)
{
  std::cerr << fhg::util::current_exception_printer() << "\n";
  return EXIT_FAILURE;
}
