#include <logging/demultiplexer.hpp>

#include <rif/started_process_promise.hpp>

#include <util-generic/print_exception.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/syscall/process_signal_block.hpp>
#include <util-generic/syscall/signal_set.hpp>

#include <cstdlib>
#include <exception>
#include <iostream>

int main (int argc, char** argv)
try
{
  fhg::rif::started_process_promise promise (argc, argv);
  try
  {
    fhg::util::syscall::signal_set const signals {SIGINT, SIGTERM};
    fhg::util::syscall::process_signal_block const signal_block (signals);

    fhg::logging::demultiplexer const actual (promise, argc, argv);

    fhg::util::syscall::sigwaitinfo (&signals._, nullptr);

    return EXIT_SUCCESS;
  }
  catch (...)
  {
    promise.set_exception (std::current_exception());
    return EXIT_FAILURE;
  }
}
catch (...)
{
  std::cerr << fhg::util::current_exception_printer() << "\n";
  return EXIT_FAILURE;
}
