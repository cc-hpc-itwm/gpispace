#include <fstream>
#include <iostream>

#include <fhglog/fhglog.hpp>
#include <fhglog/util.hpp>
#include <fhglog/StreamAppender.hpp>
#include <fhglog/FilteringAppender.hpp>
#include <fhglog/format.hpp>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char **argv)
{
  int filter = -1;

  po::options_description desc("options");

  desc.add_options()
    ("help,h", "this message")
    ("version,V", "print version information")
    ("dumpversion", "dump version information")
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

  if (vm.count("version"))
  {
    std::cerr << "FhgLog v" << FHGLOG_VERSION << std::endl;
    return EXIT_SUCCESS;
  }

  if (vm.count("dumpversion"))
  {
    std::cerr << FHGLOG_VERSION << std::endl;
    return EXIT_SUCCESS;
  }

  if (vm.count("help"))
  {
    std::cerr << "usage: fhglog-dump [options]" << std::endl;
    std::cerr << desc << std::endl;
    return EXIT_SUCCESS;
  }

  const std::string color (vm["color"].as<std::string>());
  const std::string fmt_string (vm["format"].as<std::string>());

  fhg::log::StreamAppender::ColorMode color_mode
    (fhg::log::StreamAppender::COLOR_OFF);
  if (color == "auto")
    color_mode = fhg::log::StreamAppender::COLOR_AUTO;
  if (color == "off")
    color_mode = fhg::log::StreamAppender::COLOR_OFF;
  if (color == "on")
    color_mode = fhg::log::StreamAppender::COLOR_ON;

  // my own output goes to stderr
  fhg::log::getLogger().addAppender
    (fhg::log::Appender::ptr_t (new fhg::log::StreamAppender( "console"
                                                            , std::clog
                                                            , fhg::log::default_format::SHORT()
                                                            , color_mode
                                                            )
                               )
    );

  fhg::log::LogLevel level (fhg::log::LogLevel::INFO);
  if (vm.count("quiet"))
  {
    level = fhg::log::LogLevel::ERROR;
  }
  else
  {
    if (filter < 1)
      level = fhg::log::LogLevel::TRACE;
    else if (filter == 1)
      level = fhg::log::LogLevel::DEBUG;
    else if (filter == 2)
      level = fhg::log::LogLevel::INFO;
    else if (filter == 3)
      level = fhg::log::LogLevel::WARN;
    else // if (filter == 4)
      level = fhg::log::LogLevel::ERROR;
  }

  fhg::log::getLogger().setLevel (level);

  std::string fmt (fmt_string);
  if      (fmt_string == "full")  fmt = fhg::log::default_format::LONG();
  else if (fmt_string == "short") fmt = fhg::log::default_format::SHORT();
  else fhg::log::check_format (fmt);

  // remote messages go to stdout
  fhg::log::Appender::ptr_t appender
    (new fhg::log::StreamAppender( "console"
                                 , std::cout
                                 , fmt
                                 , color_mode
                                 )
    );

  appender = fhg::log::Appender::ptr_t
    (new fhg::log::FilteringAppender ( appender
                                     , fhg::log::Filter::ptr_t
                                     (new fhg::log::LevelFilter(level))
                                     )
    );

  try
  {
    while (std::cin)
    {
      fhg::log::LogEvent evt;
      std::cin >> evt;
      if (std::cin.good ())
      {
        appender->append (evt);
      }
    }
  }
  catch (const std::exception& e)
  {
    LOG (ERROR, "could not dump log events: " << e.what ());
  }

  return 0;
}
