#include <iostream>

#include <csignal>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>
#include <boost/bind.hpp>

#include <gspc/net.hpp>

static bool stop_requested = false;

static void handle_stop_request (int)
{
  stop_requested = true;
}

class reply_frame_handler_t
{
public:
  reply_frame_handler_t ()
    : start (boost::posix_time::microsec_clock::universal_time ())
    , end ()
    , rtt_min (boost::posix_time::pos_infin)
    , rtt_max (boost::posix_time::neg_infin)
    , rtt_sum (0)
    , sent (0)
    , recv (0)
    , last (-1)
    , lost (0)
  {}

  int handle_frame (gspc::net::frame const &f)
  {
    ++recv;

    if (f.get_command () == "ERROR")
    {
      std::cerr << "error: "
                << gspc::net::header::get (f,"code",EPROTO)
                << ": "
                << gspc::net::header::get (f,"message",std::string("unknown"))
                << std::endl;
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

      rtt_sum += rtt.total_microseconds ();
      rtt_sqsum += rtt.total_microseconds () * rtt.total_microseconds ();

      if (rtt < rtt_min) rtt_min = rtt;
      if (rtt > rtt_max) rtt_max = rtt;

      size_t recv_ttl =
        gspc::net::header::get (f, "ttl", 0);

      std::cout << f.to_string ().size () << " bytes:"
                << " seq=" << recv_seq
                << " ttl=" << recv_ttl
                << " time=" << (rtt.total_microseconds () / 1000.0) << " ms"
                << std::endl;
    }
    return 0;
  }

  void dump_stats_to (std::ostream & os)
  {
    end = boost::posix_time::microsec_clock::universal_time ();

    double packet_loss = (double)(sent - recv)/(double)(sent);
    float rtt_avg = (float)rtt_sum / (float)(recv);

    os << std::endl << "--- ping statistics ---" << std::endl;
    os << sent << " packets transmitted, "
       << recv << " received, "
       << packet_loss*100.0  << "% packet loss, "
       << "time " << (rtt_sum/1000.0) << "ms"
       << std::endl
       << "rtt min/avg/max/mdev = "
       << (rtt_min.total_microseconds () / 1000.0)
       << "/"
       << (rtt_avg / 1000.0)
       << "/"
       << (rtt_max.total_microseconds () / 1000.0)
       << "/"
       << sqrt (((float)rtt_sqsum/recv - (rtt_avg*rtt_avg))) / (1000.0*1000.0)
       << " ms"
       << std::endl;
  }

  boost::posix_time::ptime start;
  boost::posix_time::ptime end;
  boost::posix_time::time_duration rtt_min;
  boost::posix_time::time_duration rtt_max;

  size_t rtt_sum;
  size_t rtt_sqsum;

  size_t sent;
  size_t recv;
  size_t last;
  size_t lost;
};

#define MS 1000

int main (int argc, char *argv[])
{
  std::string url;
  gspc::net::client_ptr_t client;
  size_t interval = 1000;
  long deadline = -1;
  size_t count = 0;
  size_t packetsize = 0;

  namespace po = boost::program_options;

  po::options_description desc ("options");
  desc.add_options ()
    ("help,h", "print help message")
    ("count,c", po::value<size_t>(&count)->default_value (count), "how many ECHO_REQUESTS to send")
    ("interval,I", po::value<size_t>(&interval)->default_value (interval), "time in ms between ECHO_REQUESTS")
    ("deadline,w", po::value<long>(&deadline)->default_value (deadline), "time in ms to wait for outstanding requests")
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

  if (0 == interval)
    interval = 1;

  gspc::net::initialize ();

  reply_frame_handler_t reply_handler;

  boost::system::error_code ec;

  client = gspc::net::dial (url, ec);
  client->onFrame.connect (boost::bind ( &reply_frame_handler_t::handle_frame
                                       , &reply_handler
                                       , _1
                                       )
                          );

  std::string payload;
  try
  {
    payload.resize (packetsize);
  }
  catch (std::exception const &ex)
  {
    std::cerr << "could not allocate memory: " << packetsize
              << ": " << ex.what () << std::endl;
    return 1;
  }

  std::fill (payload.begin (), payload.end (), '0');

  std::cout << "PING " << url << " " << payload.size () << " bytes of data." << std::endl;

  while (not stop_requested)
  {
    if (client->is_connected ())
    {
      using namespace gspc::net;

      frame ping ("SEND");
      ping.set_body (payload);
      header::set (ping, "sequence", reply_handler.sent);
      header::set (ping, "ttl", 255);
      header::set (ping, "reply-to", client->get_private_queue ());
      header::set (ping, "destination", "/service/echo");

      header::set (ping, "timestamp", boost::posix_time::microsec_clock::universal_time ());

      client->send_raw (ping);
    }
    else
    {
      int rc = client->start ();
      if (rc < 0)
      {
        std::cerr << "ping: failed: " << strerror (-rc)
                  << std::endl;
      }
    }

    reply_handler.sent++;

    if (count && reply_handler.sent == count)
      break;

    usleep (interval * MS);
  }

  if (reply_handler.sent > reply_handler.recv)
  {
    boost::posix_time::time_duration deadline_time =
      boost::posix_time::milliseconds
      ( deadline == -1
      ? std::max ( 200L
                 , 2*reply_handler.rtt_max.total_microseconds () / 1000
                 )
      : deadline
      );
    boost::this_thread::sleep (deadline_time);
  }

  reply_handler.dump_stats_to (std::cerr);

  client->stop ();
  gspc::net::shutdown ();

  return reply_handler.recv > 0 ? 0 : 1;
}
