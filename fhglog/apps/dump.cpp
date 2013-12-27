#include <fhglog/appender/stream.hpp>
#include <fhglog/fhglog.hpp>
#include <fhglog/format.hpp>

#include <fhg/util/parse/position.hpp>

#include <boost/program_options.hpp>

#include <fstream>
#include <iostream>

namespace po = boost::program_options;

int main(int argc, char **argv)
{
  int filter = -1;

  po::options_description desc("options");

  desc.add_options()
    ("help,h", "this message")
    ("filter,F", po::value<int>(&filter)->default_value(filter), "filter events with a smaller level")
    ("color,c", po::value<std::string>()->default_value("auto"), "colored output")
    ( "format,f", po::value<std::string>()->default_value("short")
    , "possible values:\n"
    "  short:\t use a short logging format (eq. to \"%s: %l %p:%L - %m%n\")\n"
    "   full:\t use a long logging format (eq. to \"%t %S %l pid:%R thread:%T %p:%L (%F) - %m%n\")\n"
    " custom:\t provide your own format\n"
    "   format flags:\n"
    "      %s - log level (short)\n"
    "      %S - log level (long)\n"
    "      %p - file name\n"
    "      %P - file path\n"
    "      %F - function\n"
    "      %L - line number\n"
    "      %m - log message\n"
    "      %d - date\n"
    "      %t - timestamp\n"
    "      %T - thread id\n"
    "      %R - process id\n"
    "      %n - new line\n"
    "      %l - logger name"
    )
    ;

  po::variables_map vm;
  po::store (po::parse_command_line(argc, argv, desc), vm);
  po::notify (vm);

  if (vm.count("help"))
  {
    std::cerr << "usage: fhglog-dump [options]" << std::endl;
    std::cerr << desc << std::endl;
    return EXIT_SUCCESS;
  }

  fhg::log::Logger::get("dump")->setLevel
    (vm.count("quiet") ? fhg::log::ERROR : fhg::log::from_int (filter));

  const std::string format_string (vm["format"].as<std::string>());

  fhg::log::Logger::get("dump")->addAppender
    ( fhg::log::Appender::ptr_t
      ( new fhg::log::StreamAppender
        ( std::cout
        , format_string == "full" ? fhg::log::default_format::LONG()
        : format_string == "short" ? fhg::log::default_format::SHORT()
        : fhg::log::check_format (format_string)
        , vm["color"].as<std::string>() == "on"
        ? fhg::log::StreamAppender::COLOR_ON
        : fhg::log::StreamAppender::COLOR_OFF
        )
      )
    );

  std::cin.unsetf (std::ios_base::skipws);
  fhg::util::parse::position_istream pos (std::cin);
  while (std::cin.good())
  {
    fhg::log::Logger::get("dump")->log (fhg::log::LogEvent (pos));
  }

  return 0;
}
