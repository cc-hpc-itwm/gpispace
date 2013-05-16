#include <iostream>

#include <sysexits.h>

#include <csignal>
#include <boost/foreach.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>

#include <gspc/net.hpp>

class reply_frame_handler_t : public gspc::net::frame_handler_t
{
public:
  reply_frame_handler_t ()
    : start (boost::posix_time::microsec_clock::universal_time ())
    , end ()
    , error_code ()
  {}

  int handle_frame (gspc::net::user_ptr, gspc::net::frame const &f)
  {
    if (f.get_command () == "ERROR")
    {
      std::cerr << "error: "
                << gspc::net::header::get (f,"code",EPROTO)
                << ": "
                << gspc::net::header::get (f,"message",std::string("unknown"))
                << std::endl;
      error_code = boost::system::errc::make_error_code
        (boost::system::errc::bad_message);
    }
    else
    {
      std::cout << f.get_body_as_string () << std::endl;
    }

    end =boost::posix_time::microsec_clock::universal_time ();
    return 0;
  }

  int handle_error (gspc::net::user_ptr, boost::system::error_code const &ec)
  {
    std::cerr << "error: " << ec << ": " << ec.message () << std::endl;
    error_code = ec;
    return 0;
  }

  boost::posix_time::ptime start;
  boost::posix_time::ptime end;

  boost::system::error_code error_code;
};

#define MS 1000

int main (int argc, char *argv[])
{
  std::string url;
  std::string body;
  std::string destination;
  std::vector<std::string> header;
  gspc::net::client_ptr_t client;
  size_t timeout = 120 * MS;
  bool show_reply_headers = false;

  namespace po = boost::program_options;

  po::options_description desc ("options");
  desc.add_options ()
    ("help,h", "print help message")
    ("body,b", po::value<std::string>(&body)->default_value (body), "body to send")
    ("timeout,T", po::value<size_t>(&timeout), "request timeout in milliseconds")
    ("url", po::value<std::string>(&url), "remote server to connect to")
    ("destination,d", po::value<std::string>(&destination), "service to request from")
    ("header,H", po::value<std::vector<std::string> >(&header), "header entries")
    ("show-reply-headers", "show headers of reply")
    ;

  po::positional_options_description p;
  p.add("url", -1);
  po::variables_map vm;
  try
  {
    po::store( po::command_line_parser(argc, argv)
             . options(desc).positional(p).run()
             , vm
             );
  }
  catch (std::exception const &ex)
  {
    std::cerr << "invalid command line: " << ex.what() << std::endl;
    std::cerr << "use " << argv [0] << " --help to get a list of options" << std::endl;
    return EXIT_FAILURE;
  }
  po::notify (vm);

  if (vm.count("help"))
  {
    std::cout << argv [0] << " [options]" << std::endl;
    std::cout << std::endl;
    std::cout << desc << std::endl;
    return EX_OK;
  }

  if (url.empty ())
  {
    std::cerr << "'url' must not be empty" << std::endl;
    return EX_USAGE;
  }

  if (destination.empty ())
  {
    std::cerr << "'destination' must not be empty" << std::endl;
    return EX_USAGE;
  }

  show_reply_headers = vm.count ("show-reply-headers") > 0;

  reply_frame_handler_t reply_handler;

  try
  {
    client = gspc::net::dial (url);
    client->set_frame_handler (reply_handler);
  }
  catch (std::exception const &ex)
  {
    std::cerr << "could not connect to '" << url << "': "
              << ex.what ()
              << std::endl;
    return 1;
  }

  gspc::net::frame rqst ("SEND");
  rqst.set_body (body);

  gspc::net::header::set (rqst, "destination", destination);

  gspc::net::frame rply;
  int rc = client->request ( rqst
                           , rply
                           , boost::posix_time::milliseconds (timeout)
                           );
  if (0 == rc)
  {
    if (show_reply_headers)
    {
      gspc::net::frame::header_type header = rply.get_header ();
      BOOST_FOREACH (gspc::net::frame::header_type::value_type item, header)
      {
        std::cout << item.first << ":" << item.second << std::endl;
      }
      if (header.size ())
        std::cout << std::endl;
    }

    if (rply.get_command () == "ERROR")
    {
      rc = gspc::net::header::get (rply, "code", (int)gspc::net::E_BAD_REQUEST);

      std::cout << "failed: "
                << rc
                << ": "
                << gspc::net::header::get (rply, "message", std::string ())
                << ": "
                << rply.get_body_as_string ()
                << std::endl
        ;
    }
    else
    {
      std::cout << rply.get_body_as_string ();
    }
  }
  else
  {
    std::cerr << "failed: " << rc << ": ";
    if (rc < 0)
    {
      std::cerr << strerror (-rc);
    }
    else
    {
      std::cerr << gspc::net::error_string ((gspc::net::error_code_t)(rc));
    }
    std::cerr << std::endl;
  }

  return rc;
}
