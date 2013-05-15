#include <iostream>

#include <csignal>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>

#include <gspc/net.hpp>

static bool stop_requested = false;

static void handle_stop_request (int)
{
  stop_requested = true;
}

class reply_frame_handler_t : public gspc::net::frame_handler_t
{
public:
  reply_frame_handler_t ()
    : start (boost::posix_time::microsec_clock::universal_time ())
    , end ()
    , rtt_min (boost::posix_time::pos_infin)
    , rtt_max (boost::posix_time::neg_infin)
    , rtt_avg (boost::posix_time::pos_infin)
    , sent (0)
    , recv (0)
    , last (-1)
    , lost (0)
    , had_error (false)
  {}

  int handle_frame (gspc::net::user_ptr, gspc::net::frame const &f)
  {
    ++recv;

    if (f.get_command () == "ERROR")
    {
      std::cerr << "error: "
                << gspc::net::header::get (f,"code",EPROTO)
                << ": "
                << gspc::net::header::get (f,"message",std::string("unknown"))
                << std::endl;
      had_error = true;
    }
    else
    {
      size_t recv_seq =
        gspc::net::header::get (f, "sequence", -1);
      boost::posix_time::ptime sent_t =
        gspc::net::header::get (f, "timestamp", end);
      boost::posix_time::ptime now =
        boost::posix_time::microsec_clock::universal_time ();
      boost::posix_time::time_duration rtt = now - sent_t;

      if (rtt < rtt_min) rtt_min = rtt;
      if (rtt > rtt_max) rtt_max = rtt;

      if (rtt_avg.is_pos_infinity ())
      {
        rtt_avg = rtt;
      }
      else
      {
        rtt_avg = (rtt + rtt_avg*recv) / (recv+1);
      }

      size_t recv_ttl =
        gspc::net::header::get (f, "ttl", 0);

      std::cout << f.to_string ().size () << " bytes:"
                << " seq=" << recv_seq
                << " ttl=" << recv_ttl
                << " time=" << rtt
                << std::endl;
    }
    return 0;
  }

  int handle_error (gspc::net::user_ptr, boost::system::error_code const &ec)
  {
    std::cerr << "error: " << ec << ": " << ec.message () << std::endl;
    had_error = true;
    return 0;
  }

  void dump_stats_to (std::ostream & os)
  {
    end = boost::posix_time::microsec_clock::universal_time ();

    double packet_loss = (double)(sent - recv)/(double)(sent);

    os << "--- ping statistics ---" << std::endl;
    os << sent << " packets transmitted, "
       << recv << " packets received, "
       << packet_loss*100.0  << "% packet loss, "
       << "time " << (end - start)
       << std::endl
       << "rtt min/avg/max = "
       << rtt_min << "/" << rtt_avg << "/" << rtt_max
       << std::endl;
  }

  boost::posix_time::ptime start;
  boost::posix_time::ptime end;
  boost::posix_time::time_duration rtt_min;
  boost::posix_time::time_duration rtt_max;
  boost::posix_time::time_duration rtt_avg;

  size_t sent;
  size_t recv;
  size_t last;
  size_t lost;
  bool   had_error;
};

#define MS 1000

int main (int argc, char *argv[])
{
  std::string url;
  gspc::net::client_ptr_t client;
  size_t interval = 1000;
  size_t deadline = 500;
  size_t count = 0;
  size_t packetsize = 0;

  namespace po = boost::program_options;

  po::options_description desc ("options");
  desc.add_options ()
    ("help,h", "print help message")
    ("count,c", po::value<size_t>(&count)->default_value (count), "how many ECHO_REQUESTS to send")
    ("interval,I", po::value<size_t>(&interval)->default_value (interval), "time in ms between ECHO_REQUESTS")
    ("deadline,w", po::value<size_t>(&deadline)->default_value (deadline), "time in ms to wait for outstanding requests")
    ("url", po::value<std::string>(&url), "remote server to ping")
    ("packetsize,s", po::value<size_t>(&packetsize)->default_value (packetsize), "simulate a body with that many bytes")
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
    return EXIT_SUCCESS;
  }

  signal (SIGTERM, handle_stop_request);
  signal (SIGINT, handle_stop_request);

  if (interval < 200 && getuid () != 0)
    interval = 200;

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

  std::string payload;
  payload.resize (packetsize);
  std::fill (payload.begin (), payload.end (), '0');

  while (not stop_requested && not reply_handler.had_error)
  {
    using namespace gspc::net;

    frame ping ("SEND");
    ping.set_body (payload);
    header::set (ping, "sequence", reply_handler.sent++);
    header::set (ping, "ttl", 255);
    header::set (ping, "reply-to", client->get_private_queue ());
    header::set (ping, "destination", "/service/echo");

    header::set (ping, "timestamp", boost::posix_time::microsec_clock::universal_time ());

    client->send_raw (ping);

    if (count && reply_handler.sent == count)
      break;

    usleep (interval * MS);
  }

  if (reply_handler.sent > reply_handler.recv)
  {
    usleep (deadline * MS);
  }

  reply_handler.dump_stats_to (std::cerr);

  return 0;
}
