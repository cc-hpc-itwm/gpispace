// alexander.petry@itwm.fraunhofer.de

#include <fhglog/appender/stream.hpp>
#include <fhglog/fhglog.hpp>
#include <fhglog/format.hpp>
#include <fhglog/remote/server.hpp>

#include <boost/program_options.hpp>

#include <csignal>
#include <cstdlib>
#include <sstream>

namespace
{
  boost::asio::io_service io_service;

  void signal_handler(int)
  {
    io_service.stop();
  }
}

int main(int argc, char **argv)
try
{
  namespace po = boost::program_options;

  po::options_description desc("options");

  desc.add_options()
    ("help,h", "this message")
    ("port,p", po::value<unsigned short>(), "port to listen on")
    ("quiet,q", "be quiet")
    ("verbose,v", po::value<unsigned int>()->default_value(0), "verbosity level")
    ("color,c", po::value<std::string>()->default_value("off"), "colored output")
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
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help"))
  {
    std::cerr << "usage: fhglog-server [options]" << std::endl;
    std::cerr << desc << std::endl;
    return EXIT_SUCCESS;
  }

  fhg::log::Logger::ptr_t l (fhg::log::Logger::get ("server"));

  l->setLevel (fhg::log::TRACE);

  const std::string format_string (vm["format"].as<std::string>());

  l->addAppender
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

  fhg::log::remote::LogServer const
    server (l, io_service, vm["port"].as<unsigned short>());

  signal (SIGINT, signal_handler);
  signal (SIGTERM, signal_handler);

  io_service.run();

  return 0;
}
catch (const std::exception& e)
{
  std::cerr << "Exception occured: " << e.what() << std::endl;

  return EXIT_FAILURE;
}
