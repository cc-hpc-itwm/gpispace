#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <string>

#include <fhglog/minimal.hpp>
#include <boost/program_options.hpp>

#include <gpi-space/signal_handler.hpp>
#include <gpi-space/pc/client/api.hpp>

namespace po = boost::program_options;

static bool read_user_input ( const std::string & prompt
                            , std::string &
                            );
static std::string const & version ();

int main (int ac, char **av)
{
  gpi::signal::handler().start();

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

  int err;
  std::string prompt;
  std::string line;
  char buf [2048];

  gpi::pc::client::api_t capi (path);

  try
  {
    capi.start ();
  }
  catch (std::exception const & ex)
  {
    std::cerr << "could not connect to " << path << ": " << ex.what() << std::endl;
  }

  for (;;)
  {
    if (capi.is_connected ())
      prompt = "gpish+> ";
    else
      prompt = "gpish-> ";
    if (! read_user_input (prompt, line))
      break;

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
      if (! capi.is_connected ())
      {
        try
        {
          capi.start ();
        }
        catch (std::exception const & ex)
        {
          std::cerr << "could not open connection to "
                    << path
                    << ": "
                    << ex.what()
                    << std::endl;
        }
      }
    }
    else if (line == "close")
    {
      capi.stop ();
    }
    else if (capi.is_connected())
    {
      // send request to gpi
      err = capi.write (line.c_str(), line.size());
      if (err < 0)
      {
        std::cerr << "could not send request to gpi: " << strerror(errno) << std::endl;
      }
      else if (err == 0)
      {
        // connection is still alive
      }
      else
      {
        memset (buf, 0, sizeof(buf));
        // wait for a reply
        err = capi.read (buf, sizeof(buf));
        if (err < 0)
        {
          std::cerr << "could not read reply from gpi: " << strerror(errno) << std::endl;
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

  capi.stop ();
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
