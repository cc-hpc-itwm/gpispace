// bernd.loerwald@itwm.fraunhofer.de

#include <rif/execute_and_get_startup_messages.hpp>

#include <fhg/util/boost/program_options/validators.hpp>
#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <fhg/util/print_exception.hpp>
#include <fhg/util/split.hpp>

#include <boost/any.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <iostream>
#include <list>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
  namespace option
  {
    constexpr const char* const startup_messages_pipe_option {"startup-messages-pipe-option"};
    constexpr const char* const end_sentinel_value {"end-sentinel-value"};
    constexpr const char* const command {"command"};
    constexpr const char* const arguments {"arguments"};
    constexpr const char* const environment {"environment"};

    struct environment_variables_t
      : std::unordered_map<std::string, std::string> {};
    void validate ( boost::any& v
                  , std::vector<std::string> const& values
                  , environment_variables_t*
                  , int
                  )
    {
      if (v.empty())
      {
        v = environment_variables_t();
      }

      environment_variables_t* tv (boost::any_cast<environment_variables_t> (&v));

      for (std::string const& value : values)
      {
        std::list<std::string> const parts
          (fhg::util::split<std::string, std::string> (value, '='));
        if (parts.size() != 2)
        {
          throw boost::program_options::invalid_option_value
            ((boost::format ("%1% not of form $key=$value") % value).str());
        }

        if (!tv->emplace (parts.front(), parts.back()).second)
        {
          throw boost::program_options::invalid_option_value
            ( ( boost::format ("multiple values for '%1%': '%2%' and '%3'")
              % parts.front()
              % parts.back()
              % tv->at (parts.front())
              ).str()
            );
        }
      }
    }
  }
}

int main (int argc, char** argv)
try
{
  boost::program_options::options_description options_description;
  options_description.add_options()
    ( option::startup_messages_pipe_option
    , boost::program_options::value<std::string>()->required()
    , "command line option to pass pipe file descriptor to child"
    )
    ( option::end_sentinel_value
    , boost::program_options::value<std::string>()->required()
    , "sentinel value last written to pipe before successfully closing pipe"
    )
    ( option::command
    , boost::program_options::value
        <fhg::util::boost::program_options::existing_path>()->required()
    , "path of command to execute"
    )
    ( option::arguments
    , boost::program_options::value<std::vector<std::string>>()->multitoken()
    , "arguments to pass to the command"
    )
    ( option::environment
    , boost::program_options::value
        <option::environment_variables_t>()->multitoken()
    , "environment variables to pass to the command"
    )
    ;

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser (argc, argv)
    . options (options_description)
    . run()
    , vm
    );
  boost::program_options::notify (vm);

  std::pair<pid_t, std::vector<std::string>> pid_and_messages
    ( fhg::rif::execute_and_get_startup_messages
        ( vm[option::startup_messages_pipe_option].as<std::string>()
        , vm[option::end_sentinel_value].as<std::string>()
        , boost::filesystem::canonical
            ( vm[option::command]
              .as<fhg::util::boost::program_options::existing_path>()
            )
        , vm.count (option::arguments)
        ? vm[option::arguments].as<std::vector<std::string>>()
        : std::vector<std::string>()
        , vm.count (option::environment)
        ? vm[option::environment].as<option::environment_variables_t>()
        : option::environment_variables_t()
        )
    );

  std::cout << pid_and_messages.first << "\n";
  for (std::string message : pid_and_messages.second)
  {
    std::cout << message << "\n";
  }

  return 0;
}
catch (...)
{
  fhg::util::print_current_exception (std::cerr, "EXCEPTION: ");
  return 1;
}
