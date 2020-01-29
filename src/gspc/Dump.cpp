#include <gspc/serialization.hpp>
#include <gspc/workflow_engine/State.hpp>

#include <util-generic/print_exception.hpp>
#include <util-generic/read_file.hpp>

#include <iostream>

int main()
try
{
  auto state ( gspc::bytes_load<gspc::workflow_engine::State>
                 (fhg::util::read_file<std::vector<char>> ("/dev/stdin"))
             );

  std::cout << state.processing_state << std::endl;

  return EXIT_SUCCESS;
}
catch (...)
{
  std::cerr << "EXCEPTION: " << fhg::util::current_exception_printer() << '\n';

  return EXIT_FAILURE;
}
