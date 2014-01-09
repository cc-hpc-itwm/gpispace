// alexander.petry@itwm.fraunhofer.de

#include <fhglog/fhglog.hpp>
#include <fhglog/remote/appender.hpp>

#include <boost/foreach.hpp>
#include <boost/program_options.hpp>

#include <iostream>
#include <sstream>

namespace po = boost::program_options;

int main (int argc, char **argv) try
{
  using namespace fhg::log;

  po::options_description desc("options");

  std::string url (getenv("FHGLOG_to_server") ? getenv("FHGLOG_to_server") : "");
  std::string file ("fhglog-client.cpp");
  std::string function ("(main)");
  int line (0);
  std::string message("-");
  std::vector<std::string> tags;
  int level (INFO);

  desc.add_options()
    ("help,h", "this message")
    ("url,U", po::value<std::string>(&url)->default_value(url), "url (host[:port]) to use")
    ("message,m", po::value<std::string>(&message), "message to send, if message is - stdin is used")
    ("priority,p", po::value<int>(&level)->default_value(level), "log level to use (0-5)")
    ("tag,t", po::value<std::vector<std::string> >(&tags), "category tag of the message")
    ("file,f", po::value<std::string>(&file)->default_value(file), "filename of the event")
    ("function,F", po::value<std::string>(&function)->default_value(function), "ffunction to set")
    ("line,L", po::value<int>(&line)->default_value(line), "line number of the event")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help"))
  {
    std::cerr << "usage: " << argv[0] << " [options]" << std::endl;
    std::cerr << desc << std::endl;
    return EXIT_SUCCESS;
  }

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

  remote::RemoteAppender r (url);
  r.append (LogEvent ( from_int (level)
                     , file
                     , function
                     , line
                     , message
                     , tags
                     )
           );

  return 0;
}
catch (std::exception const & ex)
{
  std::cerr << "Exception: " << ex.what() << std::endl;
  return 1;
}
