#include <fhglog/appender/stream.hpp>
#include <fhglog/LogMacros.hpp>
#include <fhglog/format.hpp>

#include <fhg/util/parse/position.hpp>
#include <fhg/util/print_exception.hpp>

#include <boost/program_options.hpp>

#include <fstream>
#include <iostream>

namespace po = boost::program_options;

namespace
{
  namespace option
  {
    constexpr char const* const level ("level");
    constexpr char const* const format ("format");
  }
}

int main(int argc, char **argv)
try
{
  po::options_description desc("options");

  desc.add_options()
    ("help,h", "this message")
    (option::level, po::value<std::string>()->required(), "filter events with a smaller level")
    (option::format, po::value<std::string>()->default_value("short")
    , ("possible values:\n"
    "  short:\t use a short logging format (" + fhg::log::default_format::SHORT() + ")\n"
    "   long:\t use a long logging format (" + fhg::log::default_format::LONG() + ")\n"
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
    "      %l - logger name").c_str()
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

  fhg::log::Logger::ptr_t logger (fhg::log::Logger::get ("dump"));
  logger->setLevel
    (fhg::log::from_string (vm.at (option::level).as<std::string>()));

  const std::string format_string (vm.at (option::format).as<std::string>());

  logger->addAppender
    ( fhg::log::Appender::ptr_t
      ( new fhg::log::StreamAppender
        ( std::cout
        , format_string == "long" ? fhg::log::default_format::LONG()
        : format_string == "short" ? fhg::log::default_format::SHORT()
        : fhg::log::check_format (format_string)
        )
      )
    );

  std::cin.unsetf (std::ios_base::skipws);
  fhg::util::parse::position_istream pos (std::cin);
  while (std::cin.good())
  {
    logger->log (fhg::log::LogEvent (pos));
  }

  return 0;
}
catch (...)
{
  fhg::util::print_current_exception (std::cerr, "EX: ");
  return 1;
}
