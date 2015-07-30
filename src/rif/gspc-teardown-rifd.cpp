
#include <rif/entry_point.hpp>

#include <network/connectable_to_address_string.hpp>
#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <util-generic/join.hpp>
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
    , ("strategy: one of " + fhg::util::join (strategies, ", ").string()).c_str()
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

  std::unordered_map<std::string, fhg::rif::entry_point> entry_points;

  for ( std::string line
      : fhg::util::read_lines
          ( vm.at (option::entry_points_file)
          . as<fhg::util::boost::program_options::existing_path>()
          )
      )
  {
    std::string::size_type const pos (line.find_first_of (' '));

    if (pos == std::string::npos)
    {
      std::logic_error ("Failed to parse entry_points_file");
    }

    entry_points.emplace ( line.substr (0, pos)
                         , line.substr (pos + 1, std::string::npos)
                         );
  }

  std::unordered_map<std::string, fhg::rif::entry_point> failed_entry_points;

  try
  {
    fhg::rif::strategy::teardown (strategy, entry_points, failed_entry_points);
  }
  catch (...)
  {
    for ( std::pair<std::string, fhg::rif::entry_point> const& entry_point
        : failed_entry_points
        )
    {
      std::cout << entry_point.first << ' ' << entry_point.second << '\n';
    }

    throw;
  }

  return 0;
}
catch (...)
{
  std::cerr << "EX: " << fhg::util::current_exception_printer() << '\n';

  return 1;
}
