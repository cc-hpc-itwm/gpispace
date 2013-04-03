#include <iostream>

#include <cstdio>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#include <vector>
#include <gspc/net.hpp>
#include <gspc/net/parse/parser.hpp>

static struct termios oldt;

static void restore_term_attrs ()
{
  tcsetattr (STDIN_FILENO, TCSANOW, &oldt);
}

static const char EOF_BYTE = 0x04;

class print_frame_handler_t : public gspc::net::frame_handler_t
{
public:
  bool has_error;

  int handle_frame (gspc::net::user_ptr, gspc::net::frame const &f)
  {
    if (f.get_command ().size ())
    {
      std::cout << "'";
      std::cout << f;
      std::cout << "'" << std::endl;
    }
    return 0;
  }

  int handle_error (gspc::net::user_ptr, boost::system::error_code const &ec)
  {
    std::cerr << "error: " << ec << std::endl;
    has_error = true;
    return 0;
  }
};

int main (int argc, char *argv[])
{
  struct termios newt;
  std::string url;
  gspc::net::parse::parser parser;
  gspc::net::frame frame;
  gspc::net::client_ptr_t client;

  if (argc == 1)
  {
    std::cerr << "usage: gspcnetc url" << std::endl;
    return 1;
  }

  url = argv [1];

  print_frame_handler_t print_handler;

  try
  {
    client = gspc::net::dial (url);
    print_handler.has_error = false;
    client->set_frame_handler (print_handler);
  }
  catch (std::exception const &ex)
  {
    std::cerr << "could not connect to '" << url << "': "
              << ex.what ()
              << std::endl;
    return 1;
  }

  // put stdin into raw mode
  tcgetattr (STDIN_FILENO, &oldt);
  atexit (&restore_term_attrs);

  newt = oldt;
  newt.c_lflag &= ~(ICANON);

  tcsetattr (STDIN_FILENO, TCSANOW, &newt);

  char c;

  while ((c = getchar ()) != EOF_BYTE)
  {
    if (print_handler.has_error)
    {
      try
      {
        client = gspc::net::dial (url);
      }
      catch (std::exception const &ex)
      {
        std::cerr << "could not connect to '" << url << "': "
                  << ex.what ()
                  << std::endl;
        return 1;
      }

      print_handler.has_error = false;
      client->set_frame_handler (print_handler);
    }

    gspc::net::parse::result_t result = parser.parse (&c, &c + 1, frame);
    if (result.state == gspc::net::parse::PARSE_FAILED)
    {
      std::cerr << "parse failed: " << gspc::net::error_string (result.reason)
                << std::endl;
      std::cerr << "invalid input: " << (int)(c)
                << std::endl;
    }
    else if (result.state == gspc::net::parse::PARSE_FINISHED)
    {
      if (frame.get_command ().size ())
        std::cout << std::endl;
      client->send_raw (frame);
    }
    else
    {
      continue;
    }

    parser.reset ();
    frame = gspc::net::frame ();
  }
}
