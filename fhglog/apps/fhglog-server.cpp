/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  log server daemon
 *
 *        Version:  1.0
 *        Created:  10/19/2009 09:31:40 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <sstream>
#include <cstdlib>
#include <csignal>

#include <fhglog/fhglog.hpp>
#include <fhglog/util.hpp>
#include <fhglog/remote/LogServer.hpp>
#include <fhglog/StreamAppender.hpp>
#include <fhglog/FilteringAppender.hpp>
#include <fhglog/format.hpp>
#include <boost/program_options.hpp>

boost::asio::io_service io_service;
namespace po = boost::program_options;

void signal_handler(int)
{
  io_service.stop();
}

int main(int argc, char **argv)
{
  po::options_description desc("options");

  desc.add_options()
    ("help,h", "this message")
    ("port,p", po::value<unsigned short>()->default_value(FHGLOG_DEFAULT_PORT), "port to listen on")
    ("version,V", "print version information")
    ("dumpversion", "dump version information")
    ("quiet,q", "be quiet")
    ("verbose,v", po::value<unsigned int>()->default_value(0), "verbosity level")
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
    std::cerr << "FhgLog Server v" << FHGLOG_VERSION << std::endl;
    return EXIT_SUCCESS;
  }

  if (vm.count("dumpversion"))
  {
    std::cerr << FHGLOG_VERSION << std::endl;
    return EXIT_SUCCESS;
  }

  if (vm.count("help"))
  {
    std::cerr << "usage: fhglog-server [options]" << std::endl;
    std::cerr << desc << std::endl;
    return EXIT_SUCCESS;
  }

  const std::string color (vm["color"].as<std::string>());
  const unsigned short port (vm["port"].as<unsigned short>());
  const std::string fmt_string (vm["format"].as<std::string>());

  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

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
    switch (vm["verbose"].as<unsigned int> ())
    {
    case 0:
      level = fhg::log::LogLevel::WARN;
      break;
    case 1:
      level = fhg::log::LogLevel::INFO;
      break;
    case 2:
      level = fhg::log::LogLevel::DEBUG;
      break;
    default:
      level = fhg::log::LogLevel::TRACE;
      break;
    }
  }

  fhg::log::getLogger().setLevel (level);

  std::string fmt (fmt_string);
  if      (fmt_string == "full")  fmt = fhg::log::default_format::LONG();
  else if (fmt_string == "short") fmt = fhg::log::default_format::SHORT();
  else
  {
    try
    {
      fhg::log::check_format (fmt);
    }
    catch (std::exception const &ex)
    {
      std::cerr << "invalid format: " << ex.what () << std::endl;
      return EXIT_FAILURE;
    }
  }

  // remote messages go to stdout
  fhg::log::Appender::ptr_t appender
    (new fhg::log::StreamAppender( "console"
                                 , std::cout
                                 , fmt
                                 , color_mode
                                 )
    );

  appender = fhg::log::Appender::ptr_t
    ( new fhg::log::FilteringAppender ( appender
                                      , fhg::log::Filter::ptr_t
                                      (new fhg::log::LevelFilter(level))
                                      )
    );

  try
  {
    fhg::log::remote::LogServer *server
      (new fhg::log::remote::LogServer(appender, io_service, port));
    io_service.run();
    LOG(INFO, "done.");
    delete server;
  }
  catch (const std::exception& e)
  {
    std::cerr << "Exception occured: " << e.what() << std::endl;
  }

  return 0;
}
