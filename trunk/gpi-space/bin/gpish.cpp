#include <unistd.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <string>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/shared_ptr.hpp>

#include <fhglog/minimal.hpp>

#include <gpi-space/signal_handler.hpp>
#include <gpi-space/config/parser.hpp>
#include <gpi-space/pc/client/api.hpp>
#include <gpi-space/pc/segment/segment.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

static bool read_user_input ( const std::string & prompt
                            , std::string &
                            );
static std::string const & version ();

typedef std::vector<boost::filesystem::path> path_list_t;
typedef boost::shared_ptr<gpi::pc::segment::segment_t> segment_ptr;
typedef std::vector<segment_ptr> segment_list_t;

static path_list_t collect_sockets (fs::path const & prefix);
std::ostream & operator << (std::ostream &os, path_list_t const & pl)
{
  size_t count (0);
  for ( path_list_t::const_iterator p (pl.begin())
      ; p != pl.end()
      ; ++p
      )
  {
    os << "[" << count++ << "] " << *p << std::endl;
  }
  return os;
}

int main (int ac, char **av)
{
  gpi::signal::handler().start();

  FHGLOG_SETUP();

  po::options_description desc("options");

  fs::path socket_path;
  fs::path config_file
    (std::string(getenv("HOME")) + "/.sdpa/configs/gpi.rc");

  desc.add_options ()
    ("help,h", "this message")
    ("socket,s", po::value<fs::path>(&socket_path), "path to the gpi socket")
    ("config,c", po::value<fs::path>(&config_file)->default_value(config_file), "path to the config file")
    ("list,l", "list available sockets")
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

  gpi_space::parser::config_parser_t cfg_parser;
  if (fs::exists (config_file))
  {
    gpi_space::parser::parse (config_file.string(), boost::ref(cfg_parser));
  }

  if (vm.count ("list"))
  {
    std::cout
      << collect_sockets(cfg_parser.get("gpi.socket_path", "/var/tmp"));
    return EXIT_SUCCESS;
  }

  if (socket_path.empty())
  {
    path_list_t sockets (collect_sockets(cfg_parser.get("gpi.socket_path", "/var/tmp")));
    if (sockets.empty())
    {
      std::cerr << "no sockets available!" << std::endl;
    }
    else if (sockets.size () > 1)
    {
      std::cerr << "There are multiple sockets available: " << std::endl;
      std::cerr << std::endl;
      std::cerr << sockets;
    }
    else
    {
      socket_path = *sockets.begin();
    }
  }

  std::string prompt;
  std::string line;

  segment_list_t segments;
  gpi::pc::client::api_t capi (socket_path.string());

  if (fs::exists (socket_path))
  {
    try
    {
      capi.start ();
    }
    catch (std::exception const & ex)
    {
      std::cerr << "could not connect to " << socket_path << ": " << ex.what() << std::endl;
    }
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
    else if (line.find ("open") != std::string::npos)
    {
      line = line.substr(4, line.size() - 4);
      boost::algorithm::trim (line);
      fs::path new_socket(line);
      if ( fs::exists(new_socket)
         && (new_socket.string() != capi.path())
         )
      {
        capi.path (new_socket.string());
        capi.stop ();
      }

      if (! capi.is_connected ())
      {
        try
        {
          capi.start ();
        }
        catch (std::exception const & ex)
        {
          std::cerr << "could not open connection to "
                    << capi.path ()
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
      if (line == "alloc")
      {
        try
        {
          gpi::pc::type::handle_id_t hdl
            (capi.alloc (gpi::pc::type::segment::SEG_LOCAL, 1024, "temporary"));
          std::cout << "handle = " << hdl;
        }
        catch (std::exception const & ex)
        {
          std::cerr << "failed: " << ex.what () << std::endl;
        }
      }
      else if (line == "segment")
      {
        segment_ptr seg (new gpi::pc::segment::segment_t("gpish", (128 * (1 << 20))));

        try
        {
          seg->create ();

          gpi::pc::type::segment_id_t id = capi.register_segment ( seg->name()
                                                                 , seg->size()
                                                                 );
          seg->assign_id (id);
          std::cout << "segment id: " << id << std::endl;

          segments.push_back (seg);
        }
        catch (std::exception const & ex)
        {
        std::cerr << "failed: " << ex.what() << std::endl;
        }
      }
      else if (line == "list")
      {
        try
        {
          gpi::pc::type::segment::list_t segments(capi.list_segments());

          std::cout << "segments..." << std::endl;
          for ( gpi::pc::type::segment::list_t::const_iterator it (segments.begin())
              ; it != segments.end()
              ; ++it
              )
          {
            std::cout << *it << std::endl;
          }

          gpi::pc::type::handle::list_t handles (capi.list_allocations(gpi::pc::type::segment::SEG_INVAL));
          std::cout << "allocations..." << std::endl;
          for ( gpi::pc::type::handle::list_t::const_iterator hdl (handles.begin())
              ; hdl != handles.end()
              ; ++hdl
              )
          {
            std::cout << *hdl << std::endl;
          }
        }
        catch (std::exception const & ex)
        {
          std::cerr << "list failed: " << ex.what () << std::endl;
        }
      }
      else
      {
        std::cerr << "Invalid command!" << std::endl;
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

static path_list_t collect_sockets (fs::path const & prefix)
{
  namespace fs = boost::filesystem;
  fs::path socket_path (prefix);
  socket_path /= ("GPISpace-" + boost::lexical_cast<std::string>(getuid()));

  path_list_t paths;
  if (!fs::exists (socket_path))
    return paths;

  fs::directory_iterator end_itr;
  for ( fs::directory_iterator itr (socket_path)
      ; itr != end_itr
      ; ++itr
      )
  {
    if (fs::is_other (itr->status()))
    {
      paths.push_back (itr->path());
    }
  }
  return paths;
}

