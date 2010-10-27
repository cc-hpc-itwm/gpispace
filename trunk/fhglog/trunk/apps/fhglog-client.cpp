/*
 * =====================================================================================
 *
 *       Filename:  fhglog-client.cpp
 *
 *    Description:  send messages to an fhg-log daemon
 *
 *        Version:  1.0
 *        Created:  10/19/2009 02:24:52 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <sstream> // ostringstream
#include <fhglog/fhglog.hpp>
#include <fhglog/remote/RemoteAppender.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main (int argc, char **argv)
{
  using namespace fhg::log;

  po::options_description desc("options");

  std::string url (getenv("FHGLOG_to_server") ? getenv("FHGLOG_to_server") : FHGLOG_DEFAULT_LOCATION);
  std::string file ("fhglog-client.cpp");
  int line (0);
  std::string message("-");
  std::string tag("default");
  int level (LogLevel::DEF_LEVEL);

  desc.add_options()
    ("help,h", "this message")
    ("url,U", po::value<std::string>(&url)->default_value(url), "url (host[:port]) to use")
    ("message,m", po::value<std::string>(&message), "message to send, if message is - stdin is used")
    ("priority,p", po::value<int>(&level)->default_value(level), "log level to use (0-5)")
    ("tag,t", po::value<std::string>(&tag)->default_value(tag), "category tag of the message")
    ("file,F", po::value<std::string>(&file)->default_value(file), "filename of the event")
    ("line,L", po::value<int>(&line)->default_value(line), "line number of the event")
    ("version,V", "print version information")
    ("dumpversion", "dump version information")
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
    std::cerr << "FhgLog Client v" << FHGLOG_VERSION << std::endl;
    return EXIT_SUCCESS;
  }

  if (vm.count("dumpversion"))
  {
    std::cerr << FHGLOG_VERSION << std::endl;
    return EXIT_SUCCESS;
  }

  if (vm.count("help"))
  {
    std::cerr << "usage: " << argv[0] << " [options]" << std::endl;
    std::cerr << desc << std::endl;
    return EXIT_SUCCESS;
  }

  if (level < LogLevel::MIN_LEVEL || level > LogLevel::MAX_LEVEL)
    level = LogLevel::DEF_LEVEL;

  if (message == "-")
  {
    message = "";

    do
    {
      char c;
      if (std::cin.read (&c, 1))
        message += c;
      else
        break;
    } while (true);
  }

  LogEvent e( (LogLevel::Level)level
            , file
            , ""
            , line
            , message
            );
  e.logged_via() = tag;

  try
  {
    remote::RemoteAppender r ("remote", url);
    r.append (e);
  }
  catch (std::exception const & ex)
  {
    std::cerr << "could not log message: " << ex.what() << std::endl;
    return 1;
  }

  return 0;
}
