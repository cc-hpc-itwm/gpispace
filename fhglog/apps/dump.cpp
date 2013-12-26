#include <fstream>
#include <iostream>

#include <fhglog/fhglog.hpp>
#include <fhglog/appender/stream.hpp>
#include <fhglog/format.hpp>

#include <fhg/util/parse/position.hpp>

#include <boost/program_options.hpp>

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
  try
  {
    po::store(po::parse_command_line(argc, argv, desc), vm);
  }
  catch (std::exception const & ex)
  {
    std::cerr << "invalid argument: " << ex.what() << std::endl;
    std::cerr << "try " << argv[0] << " -h to get some help" << std::endl;
    return EXIT_FAILURE;
  }
  po::notify(vm);

  if (vm.count("help"))
  {
    std::cerr << "usage: fhglog-dump [options]" << std::endl;
    std::cerr << desc << std::endl;
    return EXIT_SUCCESS;
  }

  const std::string fmt_string (vm["format"].as<std::string>());

  fhg::log::getLogger("dump").setLevel
    (vm.count("quiet") ? fhg::log::ERROR : fhg::log::from_int (filter));

  std::string fmt (fmt_string);
  if      (fmt_string == "full")  fmt = fhg::log::default_format::LONG();
  else if (fmt_string == "short") fmt = fhg::log::default_format::SHORT();
  else fhg::log::check_format (fmt);

  fhg::log::getLogger("dump").addAppender
    ( fhg::log::Appender::ptr_t
      ( new fhg::log::StreamAppender
        ( std::cout
        , fmt
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
    fhg::log::getLogger("dump").log (fhg::log::LogEvent (pos));
  }

  return 0;
}
