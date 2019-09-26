#include <logging/endpoint.hpp>
#include <logging/stdout_sink.hpp>

#include <fhg/util/boost/program_options/generic.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/syscall/process_signal_block.hpp>
#include <util-generic/syscall/signal_set.hpp>

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

  fhg::logging::stdout_sink impl (emitters.get_from (vm));

  fhg::util::syscall::sigwaitinfo (&signals._, nullptr);

  return EXIT_SUCCESS;
}
catch (...)
{
  std::cerr << fhg::util::current_exception_printer() << "\n";
  return EXIT_FAILURE;
}
