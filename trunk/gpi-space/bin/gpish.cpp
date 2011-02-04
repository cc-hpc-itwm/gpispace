#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <string>

#include <fhglog/minimal.hpp>
#include <boost/program_options.hpp>

#include <gpi-space/pc/proto/message.hpp>
#include <gpi-space/signal_handler.hpp>

namespace po = boost::program_options;

static bool connected (false);

static bool read_user_input ( const std::string & prompt
                            , std::string &
                            );
static std::string const & version ();
static int open_socket (std::string const & path);
static int close_socket (const int fd);
static int handle_sigpipe (int);

int main (int ac, char **av)
{
  gpi::signal::handler().start();
  gpi::signal::handler().connect (SIGPIPE, handle_sigpipe);

  FHGLOG_SETUP();

  po::options_description desc("options");

  std::string path;

  desc.add_options ()
    ("help,h", "this message")
    ("socket,s", po::value<std::string>(&path), "path to the gpi socket")
    ("version,V", "print version")
    ;

  po::variables_map vm;
  try
  {
    po::store(po::parse_command_line (ac,av,desc), vm);
    po::notify (vm);
  }
  catch (std::exception const & ex)
  {
    std::cerr << "invalid argument: " << ex.what() << std::endl;
    std::cerr << "try " << av[0] << " -h to get some help" << std::endl;
    return EXIT_FAILURE;
  }

  if (vm.count ("help"))
  {
    std::cerr << "usage: " << av[0] << " [options]" << std::endl;
    std::cerr << desc << std::endl;
    return EXIT_SUCCESS;
  }

  if (vm.count ("version"))
  {
    std::cerr << version () << std::endl;
    return EXIT_SUCCESS;
  }

  if (vm.count ("socket") == 0)
  {
    std::cerr << "path to socket is required!" << std::endl;
    return EXIT_FAILURE;
  }

  int socket, err;
  std::string prompt;
  std::string line;
  char buf [2048];

  // open socket
  socket = open_socket (path);

  if (socket < 0)
  {
    std::cerr << "could not connect to gpi via "
              << path
              << ": "
              << strerror(-socket)
              << std::endl;
    connected = false;
  }
  else
  {
    connected = true;
  }

  if (connected)
  {
    prompt = "gpish+> ";
  }
  else
  {
    prompt = "gpish-> ";
  }

  while (read_user_input (prompt, line))
  {
    if (connected)
    {
      prompt = "gpish+> ";
    }
    else
    {
      prompt = "gpish-> ";
    }

    // parse line into command
    if (line == "help")
    {
      std::cout << version () << std::endl;
      std::cout << "supported commands" << std::endl;
      std::cout << std::endl;
      std::cout << "   help      print this list" << std::endl;
      std::cout << "  alloc      allocate memory" << std::endl;
      std::cout << "   free      free memory" << std::endl;
      std::cout << "   list      list GPI information" << std::endl;
      std::cout << "   open      (re)open connection" << std::endl;
      std::cout << "  close      close connection" << std::endl;
      std::cout << "   exit      exit the shell" << std::endl;
    }
    else if (line == "exit")
    {
      break;
    }
    else if (line == "open")
    {
      close_socket (socket);
      socket = open_socket (path);

      if (socket < 0)
      {
        std::cerr << "could not connect to gpi via "
                  << path
                  << ": "
                  << strerror(-socket)
                  << std::endl;
        connected = false;
      }
      else
      {
        connected = true;
      }
    }
    else if (line == "close")
    {
      close_socket (socket);
      connected = false;
    }
    else if (connected)
    {
      // send request to gpi
      err = write (socket, line.c_str(), line.size());
      if (err < 0)
      {
        std::cerr << "could not send request to gpi: " << strerror(errno) << std::endl;
        close_socket (socket);
        connected = false;
      }
      else if (err == 0)
      {
        // connection is still alive
      }
      else
      {
        memset (buf, 0, sizeof(buf));
        // wait for a reply
        err = read (socket, buf, sizeof(buf));
        if (err < 0)
        {
          std::cerr << "could not read reply from gpi: " << strerror(errno) << std::endl;
          close_socket (socket);
          connected = false;
        }
        else
        {
          buf[err] = 0;
          // handle reply message
          std::cout << buf << std::endl;
        }
      }
    }
  }

  close_socket (socket);

  gpi::signal::handler().stop();
}

static std::string const & version ()
{
  static std::string ver("GPIsh 0.1");
  return ver;
}

static bool read_user_input ( const std::string & prompt
                            , std::string & s
                            )
{
  char * line (readline(prompt.c_str()));
  if (!line)
  {
    return false;
  }

  s = std::string (line);
  if (*line)
  {
    add_history(line);
  }
  free (line);
  return true;
}

static int open_socket (std::string const & path)
{
  int sfd, err;
  struct sockaddr_un addr;

  sfd = socket (AF_UNIX, SOCK_STREAM, 0);
  if (sfd < 0)
  {
    return -errno;
  }

  memset (&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy (addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

  err = connect (sfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un));
  if (err < 0)
  {
    err = -errno;
    close_socket (sfd);
    return err;
  }

  return sfd;
}

static int close_socket (const int fd)
{
  shutdown (fd, SHUT_RDWR);
  return close (fd);
}

static int handle_sigpipe (int)
{
  connected = false;
  std::cerr << "connection lost" << std::endl;
  return 0;
}
