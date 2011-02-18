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

#include <gpi-space/shell/shell.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

static std::string const & version ();

typedef std::vector<boost::filesystem::path> path_list_t;
typedef boost::shared_ptr<gpi::pc::segment::segment_t> segment_ptr;
typedef std::map<gpi::pc::type::segment_id_t, segment_ptr> segment_map_t;

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

struct my_state_t
{
  my_state_t (fs::path const & path)
    : capi (path.string())
  {}

  // handler functions
  int error (std::exception const & ex)
  {
    std::cerr << "failed: " << ex.what() << std::endl;
    return 0;
  }

  gpi::pc::client::api_t capi;
  segment_map_t segments;
};

typedef gpi::shell::basic_shell_t<my_state_t> shell_t;

static my_state_t *state (NULL);

static void initialize_state (fs::path const & socket_path);
static void initialize_shell (int ac, char *av[]);

static void shutdown_state ();
static void shutdown_shell ();

// shell functions
static int cmd_help (shell_t::argv_t const & av, shell_t & sh);
static int cmd_exit (shell_t::argv_t const & av, shell_t & sh);
static int cmd_open (shell_t::argv_t const & av, shell_t & sh);
static int cmd_close (shell_t::argv_t const & av, shell_t & sh);
static int cmd_ping (shell_t::argv_t const & av, shell_t & sh);
static int cmd_info (shell_t::argv_t const & av, shell_t & sh);

static int cmd_segment (shell_t::argv_t const & av, shell_t & sh);
static int cmd_segment_register (shell_t::argv_t const & av, shell_t & sh);
static int cmd_segment_unregister (shell_t::argv_t const & av, shell_t & sh);
static int cmd_segment_attach (shell_t::argv_t const & av, shell_t & sh);
static int cmd_segment_detach (shell_t::argv_t const & av, shell_t & sh);
static int cmd_segment_list (shell_t::argv_t const & av, shell_t & sh);

static int cmd_memory_alloc (shell_t::argv_t const & av, shell_t & sh);
static int cmd_memory_free (shell_t::argv_t const & av, shell_t & sh);
static int cmd_memory_copy (shell_t::argv_t const & av, shell_t & sh);
static int cmd_memory_wait (shell_t::argv_t const & av, shell_t & sh);
static int cmd_memory_list (shell_t::argv_t const & av, shell_t & sh);

int main (int ac, char **av)
{
  gpi::signal::handler().start();

  FHGLOG_SETUP(ac, av);

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

  initialize_state (socket_path);
  initialize_shell (ac, av);

  shell_t::get().run();

  shutdown_shell ();
  shutdown_state ();

  gpi::signal::handler().stop();
}

void initialize_state (fs::path const & socket_path)
{
  if (state) delete state;
  // set up state
  state = new my_state_t (socket_path);

  if (fs::exists (socket_path))
  {
    try
    {
      state->capi.start ();
    }
    catch (std::exception const & ex)
    {
      std::cerr << "could not connect to " << socket_path << ": " << ex.what() << std::endl;
    }
  }
}

void shutdown_state ()
{
  state->capi.stop();
  state->segments.clear();
}

void initialize_shell (int ac, char *av[])
{
  shell_t & sh (shell_t::create (av[0], "gpish> ", *state));

  sh.add_command("help", &cmd_help, "print help about commands");
  sh.add_command("exit", &cmd_exit, "exit the shell loop");
  sh.add_command("open", &cmd_open, "open connection to gpi");
  sh.add_command("close", &cmd_close, "close connection to gpi");
  sh.add_command("ping", &cmd_ping, "test connection status");
  sh.add_command("info", &cmd_info, "print information about gpi");

  sh.add_command("segment", &cmd_segment, "segment related functions");
  sh.add_command("segment-register", &cmd_segment_register, "register a new segment");
  sh.add_command("segment-unregister", &cmd_segment_unregister, "unregister a segment");
  sh.add_command("segment-attach", &cmd_segment_attach, "attach to an existing segment");
  sh.add_command("segment-detach", &cmd_segment_detach, "detach from a segment");
  sh.add_command("segment-list", &cmd_segment_list, "list available segments");

  sh.add_command("memory-alloc", &cmd_memory_alloc, "allocate memory");
  sh.add_command("memory-free", &cmd_memory_free, "deallocate memory");
  sh.add_command("memory-copy", &cmd_memory_copy, "copy memory");
  sh.add_command("memory-wait", &cmd_memory_wait, "wait for a copy to finish");
  sh.add_command("memory-list", &cmd_memory_list, "list allocations");
}

void shutdown_shell ()
{
  shell_t::destroy ();
}

int cmd_help (shell_t::argv_t const & av, shell_t & sh)
{
  if (av.empty())
  {
    // print list of commands
    const shell_t::command_list_t & cmds (sh.commands());
    for ( shell_t::command_list_t::const_iterator cmd (cmds.begin())
        ; cmd != cmds.end()
        ; ++cmd
        )
    {
      // use sh.cout()/cerr()?
      std::cout << "    " << cmd->name () << "\t" << cmd->short_doc () << std::endl;
    }
  }
  else
  {
    const shell_t::command_t *cmd (sh.find_command (av[0]));
    if (cmd)
    {
      std::cout << cmd->name() << "    " << cmd->long_doc() << std::endl;
    }
    else
    {
      std::cout << "no such command: " << av[0] << std::endl;
      return 1;
    }
  }
  return 0;
}

int cmd_exit (shell_t::argv_t const & av, shell_t & sh)
{
  sh.stop ();
  return 0;
}

int cmd_open (shell_t::argv_t const & av, shell_t & sh)
{
  if (av.size())
  {
    fs::path new_socket (av[0]);
    if ( fs::exists(new_socket)
       && (new_socket.string() != sh.state().capi.path())
       )
    {
      sh.state().capi.path (new_socket.string());
      sh.state().capi.stop ();
    }
  }

  if (! sh.state().capi.is_connected())
  {
    try
    {
      sh.state().capi.start ();
    }
    catch (std::exception const & ex)
    {
      std::cerr << "could not open connection to "
                << sh.state().capi.path ()
                << ": "
                << ex.what()
                << std::endl;
      return 1;
    }
  }
  return 0;
}

int cmd_close (shell_t::argv_t const & av, shell_t & sh)
{
  sh.state().capi.stop();
  return 0;
}

int cmd_ping (shell_t::argv_t const & av, shell_t & sh)
{
  return 1;
}

int cmd_info (shell_t::argv_t const & av, shell_t & sh)
{
  return 1;
}

int cmd_segment (shell_t::argv_t const & av, shell_t & sh)
{
  if (av.empty())
  {
    std::cout << "usage: segment [command]" << std::endl;
    std::cout << "    register" << std::endl;
    std::cout << "    unregister" << std::endl;
    std::cout << "    attach" << std::endl;
    std::cout << "    detach" << std::endl;
    std::cout << "    list" << std::endl;
    return 0;
  }
  else
  {
    shell_t::argv_t new_av (av);
    new_av[0] = "segment-" + av[0];
    return sh.execute (new_av);
  }
}

int cmd_segment_register (shell_t::argv_t const & av, shell_t & sh)
{
  // usage: create name size [flags]
  std::string name;
  gpi::pc::type::size_t size (0);
  gpi::pc::type::flags_t flags (0);

  if (av.size() < 2)
  {
    std::cerr << "usage: " << "register name size [flags]" << std::endl;
    std::cerr << "    flags:  e - exclusive" << std::endl;
    std::cerr << "            u - nounlink" << std::endl;
    std::cerr << "            p - persistent" << std::endl;
    std::cerr << "            o - do not create, open only" << std::endl;
    std::cerr << "            f - force unlink before create" << std::endl;
    return 1;
  }
  else
  {
    if (!sh.state().capi.is_connected ())
    {
      std::cerr << "not connected to gpi!" << std::endl;
      return 1;
    }

    name = av[0];
    size = boost::lexical_cast<gpi::pc::type::size_t>(av[1]);

    if (av.size() > 2)
    {
      std::string flagstring (av[2]);
      for ( std::string::const_iterator f (flagstring.begin())
          ; f != flagstring.end()
          ; ++f
          )
      {
        switch (*f)
        {
        case 'e':
          flags |= gpi::pc::type::segment::F_EXCLUSIVE;
          break;
        case 'u':
          flags |= gpi::pc::type::segment::F_NOUNLINK;
          break;
        case 'p':
          flags |= gpi::pc::type::segment::F_PERSISTENT;
          break;
        case 'o':
          flags |= gpi::pc::type::segment::F_NOCREATE;
          break;
        case 'f':
          flags |= gpi::pc::type::segment::F_FORCE_UNLINK;
          break;
        default:
          std::cerr << "invalid flag: '" << *f << "'" << std::endl;
          return 2;
        }
      }
    }

    try
    {
      segment_ptr seg (new gpi::pc::segment::segment_t(name, size));

      if (flags & gpi::pc::type::segment::F_NOCREATE)
      {
        seg->open();
      }
      else
      {
        if (flags & gpi::pc::type::segment::F_FORCE_UNLINK)
        {
          seg->unlink();
        }
        seg->create ();
      }

      gpi::pc::type::segment_id_t id = sh.state().capi.register_segment ( seg->name()
                                                                        , seg->size()
                                                                        , flags
                                                                        );
      seg->assign_id (id);
      sh.state().segments[id] = seg;

      std::cout << "segment id: " << id << std::endl;
    }
    catch (std::exception const & ex)
    {
      std::cerr << "could not register segment: " << ex.what() << std::endl;
      return 3;
    }
  }

  return 0;
}

int cmd_segment_list (shell_t::argv_t const & av, shell_t & sh)
{
  if (!sh.state().capi.is_connected ())
  {
    std::cerr << "not connected to gpi!" << std::endl;
    return 1;
  }

  try
  {
    std::cout << sh.state().capi.list_segments();
    return 0;
  }
  catch (std::exception const & ex)
  {
    std::cerr << "failed: " << ex.what () << std::endl;
    return 1;
  }
}

int cmd_segment_unregister (shell_t::argv_t const & av, shell_t & sh)
{
  if (!sh.state().capi.is_connected ())
  {
    std::cerr << "not connected to gpi!" << std::endl;
    return 1;
  }

  int err (0);
  if (av.size())
  {
    for ( shell_t::argv_t::const_iterator segment(av.begin())
        ; segment != av.end()
        ; ++segment
        )
    {
      try
      {
        gpi::pc::type::segment_id_t seg_id (boost::lexical_cast<gpi::pc::type::segment_id_t>(*segment));
        sh.state().segments.erase (seg_id);
        sh.state().capi.unregister_segment (seg_id);
      }
      catch (std::exception const & ex)
      {
        std::cerr << "unregister (" << *segment << ") failed: " << ex.what() << std::endl;
        ++err;
      }
    }
  }

  return err;
}

int cmd_segment_attach (shell_t::argv_t const & av, shell_t & sh)
{
  if (!sh.state().capi.is_connected ())
  {
    std::cerr << "not connected to gpi!" << std::endl;
    return 1;
  }

  int err (0);
  if (av.size())
  {
    for ( shell_t::argv_t::const_iterator segment(av.begin())
        ; segment != av.end()
        ; ++segment
        )
    {
      try
      {
        gpi::pc::type::segment_id_t seg_id (boost::lexical_cast<gpi::pc::type::segment_id_t>(*segment));
        sh.state().capi.attach_segment (seg_id);
      }
      catch (std::exception const & ex)
      {
        std::cerr << "attach (" << *segment << ") failed: " << ex.what() << std::endl;
        ++err;
      }
    }
  }

  return err;
}

int cmd_segment_detach (shell_t::argv_t const & av, shell_t & sh)
{
  if (!sh.state().capi.is_connected ())
  {
    std::cerr << "not connected to gpi!" << std::endl;
    return 1;
  }

  int err (0);
  if (av.size())
  {
    for ( shell_t::argv_t::const_iterator segment(av.begin())
        ; segment != av.end()
        ; ++segment
        )
    {
      try
      {
        gpi::pc::type::segment_id_t seg_id (boost::lexical_cast<gpi::pc::type::segment_id_t>(*segment));
        sh.state().capi.detach_segment (seg_id);
        sh.state().segments.erase (seg_id);
      }
      catch (std::exception const & ex)
      {
        std::cerr << "detach (" << *segment << ") failed: " << ex.what() << std::endl;
        ++err;
      }
    }
  }
  return err;
}

int cmd_memory_alloc (shell_t::argv_t const & av, shell_t & sh)
{
  return 1;
}

int cmd_memory_free (shell_t::argv_t const & av, shell_t & sh)
{
  return 1;
}
int cmd_memory_copy (shell_t::argv_t const & av, shell_t & sh)
{
  return 1;
}
int cmd_memory_wait (shell_t::argv_t const & av, shell_t & sh)
{
  return 1;
}
int cmd_memory_list (shell_t::argv_t const & av, shell_t & sh)
{
  return 1;
}

static std::string const & version ()
{
  static std::string ver("GPIsh 0.1");
  return ver;
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
