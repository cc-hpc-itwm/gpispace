
#include <rif/entry_point.hpp>

#include <network/connectable_to_address_string.hpp>
#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <fhg/util/join.hpp>
#include <util-generic/print_exception.hpp>
#include <fhg/util/read_lines.hpp>

#include <rif/strategy/meta.hpp>

#include <boost/program_options.hpp>

#include <iostream>
#include <vector>

namespace
{
  namespace option
  {
    constexpr const char* const entry_points_file {"entry-points-file"};
    constexpr const char* const strategy {"strategy"};
  }
}

int main (int argc, char** argv)
try
{
  std::vector<std::string> const strategies
    {fhg::rif::strategy::available_strategies()};

  boost::program_options::options_description options_description;
  options_description.add_options()
    ( option::entry_points_file
    , boost::program_options::value
        <fhg::util::boost::program_options::existing_path>()->required()
    , "entry_points file"
    )
    ( option::strategy
    , boost::program_options::value<std::string>()->required()
    , ("strategy: one of " + fhg::util::join (strategies, ", ")).c_str()
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

  std::vector<std::string> const lines
    ( fhg::util::read_lines
        ( vm.at (option::entry_points_file)
        . as<fhg::util::boost::program_options::existing_path>()
        )
    );
  std::vector<fhg::rif::entry_point> const entry_points
    (lines.begin(), lines.end());

  std::vector<fhg::rif::entry_point> failed_entry_points;

  try
  {
    fhg::rif::strategy::teardown (strategy, entry_points, failed_entry_points);
  }
  catch (...)
  {
    for (fhg::rif::entry_point const& entry_point : failed_entry_points)
    {
      std::cout << entry_point.to_string() << '\n';
    }

    throw;
  }

  return 0;
}
catch (...)
{
  fhg::util::print_current_exception (std::cerr, "EX: ");

  return 1;
}
