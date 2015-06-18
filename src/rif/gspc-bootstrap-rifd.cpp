
#include <rif/entry_point.hpp>

#include <fhg/util/boost/program_options/validators/existing_directory.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_file.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/join.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/read_lines.hpp>

#include <rif/strategy/meta.hpp>

#include <boost/program_options.hpp>

#include <iostream>
#include <vector>

namespace
{
  namespace option
  {
    constexpr const char* const hostfile {"hostfile"};
    constexpr const char* const port {"port"};
    constexpr const char* const strategy {"strategy"};
    constexpr const char* const gspc_home {"gspc-home"};
  }
}

int main (int argc, char** argv)
try
{
  std::vector<std::string> const strategies
    {fhg::rif::strategy::available_strategies()};

  boost::program_options::options_description options_description;
  options_description.add_options()
    ( option::port
    , boost::program_options::value
        <fhg::util::boost::program_options::positive_integral<unsigned short>>()
    , "port to listen on"
    )
    ( option::hostfile
    , boost::program_options::value
        <fhg::util::boost::program_options::nonempty_file>()->required()
    , "hostfile"
    )
    ( option::strategy
    , boost::program_options::value<std::string>()->required()
    , ("strategy: one of " + fhg::util::join (strategies, ", ")).c_str()
    )
    ( option::gspc_home
    , boost::program_options::value
        <fhg::util::boost::program_options::existing_directory>()->required()
    , "installation path of gpispace"
    )
    ;

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser (argc, argv)
      .options (options_description)
      .run()
    , vm
    );

  boost::program_options::notify (vm);

  std::string const strategy (vm.at (option::strategy).as<std::string>());

  if (std::find (strategies.begin(), strategies.end(), strategy) == strategies.end())
  {
    throw std::invalid_argument
      (( boost::format ("invalid argument '%1%' for --%2%: one of %3%")
       % strategy
       % option::strategy
       % fhg::util::join (strategies, ", ")
       ).str()
      );
  }

  for ( fhg::rif::entry_point const& entry_point
      : fhg::rif::strategy::bootstrap
          ( strategy
          , fhg::util::read_lines
              ( vm.at (option::hostfile)
              . as<fhg::util::boost::program_options::nonempty_file>()
              )
          , vm.count (option::port)
          ? boost::make_optional<unsigned short>
              ( vm.at (option::port)
              . as<fhg::util::boost::program_options::positive_integral<unsigned short>>()
              )
          : boost::none
          , boost::filesystem::canonical
              ( vm.at (option::gspc_home)
              . as<fhg::util::boost::program_options::existing_directory>()
              )
          )
      )
  {
    std::cout << entry_point.to_string() << '\n';
  }
}
catch (...)
{
  std::cerr << "EX: " << fhg::util::current_exception_printer() << '\n';

  return 1;
}
