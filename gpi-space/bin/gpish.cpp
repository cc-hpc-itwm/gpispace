#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/timer.hpp>

#include <fhglog/minimal.hpp>

#include <fhg/revision.hpp>

#include <gpi-space/signal_handler.hpp>
#include <gpi-space/pc/client/api.hpp>
#include <gpi-space/pc/type/flags.hpp>
#include <gpi-space/pc/type/handle.hpp>
#include <gpi-space/pc/segment/segment.hpp>

#include <gpi-space/shell/shell.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

static bool interactive(false);

typedef std::vector<boost::filesystem::path> path_list_t;

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
  my_state_t ( fs::path const & dir
             , fs::path const & file
             , std::size_t com_size
             )
    : socket_dir (dir)
    , capi (file.string())
    , m_com_size (com_size)
  {}

  // handler functions
  int error (std::exception const & ex)
  {
    std::cerr << "failed: " << ex.what() << std::endl;
    return 0;
  }

  int get_handle_descriptor( gpi::pc::type::handle_t const & hdl
                           , gpi::pc::type::handle::descriptor_t & desc
                           )
  {
    gpi::pc::type::handle::list_t handles
      (capi.list_allocations());
    BOOST_FOREACH(gpi::pc::type::handle::descriptor_t const &d, handles)
    {
      if (d.id == hdl)
      {
        desc = d;
        return 0;
      }
    }

    return -ESRCH;
  }

  int setup_transfer_buffer ()
  {
    if (0 == m_com_size)
      return -1;

    // register segment
    m_shm_com = capi.register_segment ( "gpish-" + boost::lexical_cast<std::string>(getpid ())
                                      , m_com_size
                                      , gpi::pc::F_EXCLUSIVE
                                      | gpi::pc::F_FORCE_UNLINK
                                      );
    m_shm_com_hdl = capi.alloc ( m_shm_com
                               , m_com_size
                               , "gpish-"+boost::lexical_cast<std::string>(getpid ())
                               , gpi::pc::F_EXCLUSIVE
                               );
    m_shm_com_ptr = (char*)capi.ptr(m_shm_com_hdl);

    return 0;
  }

  char*  com_buffer() { return m_shm_com_ptr; }
  size_t com_size() const   { return m_com_size; }
  gpi::pc::type::handle_t shm_com_hdl() const { return m_shm_com_hdl; }

  fs::path socket_dir;
  gpi::pc::client::api_t capi;
private:
  std::size_t                 m_com_size;
  gpi::pc::type::segment_id_t m_shm_com;
  gpi::pc::type::handle_t     m_shm_com_hdl;
  char                       *m_shm_com_ptr;
};

static void print_progress( FILE *fp
                          , const std::size_t current
                          , const std::size_t total
                          , const std::size_t width = 73
                          );

typedef gpi::shell::basic_shell_t<my_state_t> shell_t;

static my_state_t *state (NULL);

static int interrupt_shell (int)
{
  std::cerr << "stopping gpi api!" << std::endl;
  shell_t::get().state().capi.stop();
  return 0;
}

static void initialize_state ( fs::path const & socket_dir
                             , fs::path const & socket_path
                             , std::size_t com_size
                             );
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
static int cmd_list (shell_t::argv_t const & av, shell_t & sh);
static int cmd_socket (shell_t::argv_t const & av, shell_t & sh);
static int cmd_status (shell_t::argv_t const & av, shell_t & sh);
static int cmd_gc (shell_t::argv_t const & av, shell_t & sh);
static int cmd_set (shell_t::argv_t const & av, shell_t & sh);
static int cmd_unset (shell_t::argv_t const & av, shell_t & sh);
static int cmd_save (shell_t::argv_t const & av, shell_t & sh);
static int cmd_load (shell_t::argv_t const & av, shell_t & sh);

static int cmd_segment (shell_t::argv_t const & av, shell_t & sh);
static int cmd_segment_register (shell_t::argv_t const & av, shell_t & sh);
static int cmd_segment_unregister (shell_t::argv_t const & av, shell_t & sh);
static int cmd_segment_attach (shell_t::argv_t const & av, shell_t & sh);
static int cmd_segment_detach (shell_t::argv_t const & av, shell_t & sh);
static int cmd_segment_list (shell_t::argv_t const & av, shell_t & sh);

static int cmd_memory (shell_t::argv_t const & av, shell_t & sh);
static int cmd_memory_alloc (shell_t::argv_t const & av, shell_t & sh);
static int cmd_memory_free (shell_t::argv_t const & av, shell_t & sh);
static int cmd_memory_copy (shell_t::argv_t const & av, shell_t & sh);
static int cmd_memory_wait (shell_t::argv_t const & av, shell_t & sh);
static int cmd_memory_list (shell_t::argv_t const & av, shell_t & sh);
static int cmd_memory_add (shell_t::argv_t const & av, shell_t & sh);
static int cmd_memory_del (shell_t::argv_t const & av, shell_t & sh);

int main (int ac, char **av)
{
  gpi::signal::handler().start();

  FHGLOG_SETUP(ac, av);

  if (isatty(0))
    interactive = true;

  po::options_description desc("options");

  fs::path socket_path;
  typedef std::vector <fs::path> dir_list_t;
  dir_list_t socket_search_dir;
  socket_search_dir.push_back ("/tmp");
  socket_search_dir.push_back ("/var/tmp");

  std::size_t com_size (4 * 1024 * 1024);

  desc.add_options ()
    ("help,h", "this message")

    ("socket,s", po::value<fs::path>(&socket_path), "path to the gpi socket")

    ( "socket-dir,d"
    , po::value<dir_list_t>(&socket_search_dir)
    , "path to possible socket directories"
    )

    ( "com-size"
    , po::value<std::size_t>(&com_size)->default_value(com_size)
    , "set the size of the communication buffer, if 0, no buffer "
      "will be used at all"
    )
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
    std::cerr << fhg::project_info ("Fraunhofer GPI shell");
    return EXIT_SUCCESS;
  }

  if (vm.count ("list"))
  {
    for ( dir_list_t::iterator it = socket_search_dir.begin ()
        ; it != socket_search_dir.end ()
        ; ++it
        )
    {
      std::cout
        << collect_sockets(*it);
    }
    return EXIT_SUCCESS;
  }

  fs::path socket_dir;

  if (socket_path.empty())
  {
    for ( dir_list_t::iterator it = socket_search_dir.begin ()
        ; it != socket_search_dir.end ()
        ; ++it
        )
    {
      socket_dir = *it;
      path_list_t sockets (collect_sockets(*it));

      if (sockets.size ())
      {
        if (sockets.size () > 1)
        {
          std::cerr << "There are multiple sockets available: " << std::endl;
          std::cerr << sockets;
          std::cerr << std::endl;
        }
        socket_path = *sockets.begin ();
        break;
      }
    }
  }
  else
  {
    socket_dir = socket_path.parent_path ();
  }

  initialize_state (socket_dir, socket_path, com_size);
  initialize_shell (ac, av);

  shell_t::get().run();

  if (interactive)
    std::cerr << "logout" << std::endl;

  shutdown_shell ();
  shutdown_state ();

  gpi::signal::handler().stop();
}

void initialize_state ( fs::path const & socket_dir
                      , fs::path const & socket_file
                      , std::size_t com_size
                      )
{
  if (state) delete state;
  // set up state
  state = new my_state_t (socket_dir, socket_file, com_size);

  if (fs::exists (socket_file))
  {
    try
    {
      state->capi.start ();
      state->setup_transfer_buffer();
    }
    catch (std::exception const & ex)
    {
      std::cerr << "could not connect to " << socket_file << ": " << ex.what() << std::endl;
    }
  }
}

void shutdown_state ()
{
  state->capi.stop();
}

void initialize_shell (int ac, char *av[])
{
  std::string prompt;
  if (interactive)
    prompt = "gpish> ";
  fs::path histfile (getenv("HOME"));
  histfile /= ".gpish_history";

  shell_t & sh (shell_t::create (av[0], prompt, histfile.string(), *state));

  sh.add_command("help", &cmd_help, "print help about commands");
  sh.add_command("exit", &cmd_exit, "exit the shell loop");
  sh.add_command("open", &cmd_open, "open connection to gpi");
  sh.add_command("close", &cmd_close, "close connection to gpi");
  sh.add_command("ping", &cmd_ping, "test connection status");
  sh.add_command("info", &cmd_info, "print information about gpi");
  sh.add_command("socket", &cmd_socket, "list available sockets");
  sh.add_command("status", &cmd_status, "print internal status information");
  sh.add_command("gc", &cmd_gc, "garbage collect");
  sh.add_command("save", &cmd_save, "save a handle to disk");
  sh.add_command("load", &cmd_load, "load a file into a handle");
  sh.add_command("set", &cmd_set, "set or list shell environment");
  sh.add_command("unset", &cmd_unset, "unset variables");

  sh.add_command("segment", &cmd_segment, "segment related functions");
  sh.add_command("segment-register", &cmd_segment_register, "register a new segment");
  sh.add_command("segment-unregister", &cmd_segment_unregister, "unregister a segment");
  sh.add_command("segment-attach", &cmd_segment_attach, "attach to an existing segment");
  sh.add_command("segment-detach", &cmd_segment_detach, "detach from a segment");
  sh.add_command("segment-list", &cmd_segment_list, "list available segments");

  sh.add_command("memory", &cmd_memory, "memory related functions");
  sh.add_command("memory-alloc", &cmd_memory_alloc, "allocate memory");
  sh.add_command("memory-free", &cmd_memory_free, "deallocate memory");
  sh.add_command("memory-copy", &cmd_memory_copy, "copy memory");
  sh.add_command("memory-wait", &cmd_memory_wait, "wait for a copy to finish");
  sh.add_command("memory-list", &cmd_memory_list, "list allocations");
  sh.add_command("memory-add", &cmd_memory_add, "add memory segment");
  sh.add_command("memory-del", &cmd_memory_del, "delete memory segment");

  // TODO alias definitions
  sh.add_command("alloc", &cmd_memory_alloc, "allocate memory");
  sh.add_command("free", &cmd_memory_free, "free memory");
  sh.add_command("memcpy", &cmd_memory_copy, "copy memory");
  sh.add_command("wait", &cmd_memory_wait, "wait for copy completion");

  sh.add_command("list", &cmd_list, "list segments and allocations");
  sh.add_command("ls", &cmd_list, "list segments and allocations");

  sh.add_command ("add", &cmd_memory_add, "add a memory segment");
  sh.add_command ("del", &cmd_memory_del, "delete a memory segment");

  gpi::signal::handler().connect(SIGINT, &interrupt_shell);
}

void shutdown_shell ()
{
  shell_t::destroy ();
}

int cmd_help (shell_t::argv_t const & av, shell_t & sh)
{
  if (av.size() < 2)
  {
    std::cout << "This is a list of available commands, use 'help <command>' to get more information " << std::endl;
    std::cout << "on a particular topic." << std::endl;
    std::cout << std::endl;

    std::ios_base::fmtflags saved_flags (std::cout.flags());
    char saved_fill = std::cout.fill (' ');
    std::size_t saved_width = std::cout.width (0);

    // print list of commands
    const shell_t::command_list_t & cmds (sh.commands());
    for ( shell_t::command_list_t::const_iterator cmd (cmds.begin())
        ; cmd != cmds.end()
        ; ++cmd
        )
    {
      std::cout << "  ";
      std::cout.width (20);
      std::cout.flags (std::ios::right);
      std::cout << cmd->name();
      std::cout << "   ";
      std::cout.width (0);
      std::cout << cmd->short_doc ();
      std::cout << std::endl;
    }

    std::cout.flags (saved_flags);
    std::cout.fill (saved_fill);
    std::cout.width (saved_width);
  }
  else
  {
    const shell_t::command_t *cmd (sh.find_command (av[1]));
    if (cmd)
    {
      std::cout << cmd->name() << "    " << cmd->long_doc() << std::endl;
    }
    else
    {
      std::cout << "no such command: " << av[1] << std::endl;
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
  if (av.size() > 1)
  {
    fs::path new_socket (av[1]);
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
      state->setup_transfer_buffer();
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
  if (sh.state().capi.ping ())
  {
    std::cout << "pong" << std::endl;
    return 0;
  }
  else
  {
    std::cout << "pang" << std::endl;
    return 1;
  }
}

int cmd_info (shell_t::argv_t const & av, shell_t & sh)
{
  std::cout << sh.state().capi.collect_info () << std::endl;
  return 0;
}

int cmd_gc (shell_t::argv_t const & av, shell_t & sh)
{
  sh.state().capi.garbage_collect ();
  return 0;
}

int cmd_unset (shell_t::argv_t const & av, shell_t & sh)
{
  if (av.size() < 2)
  {
    std::cerr << "usage: unset var [var...]" << std::endl;
    return 1;
  }
  else
  {
    for (std::size_t i = 1; i < av.size(); ++i)
    {
      //sh.state.env.unset (av[i]);
    }
  }
  return 0;
}

int cmd_set (shell_t::argv_t const & av, shell_t & sh)
{
  if (av.size() < 2)
  {
    // print current state variables
//    std::cout << sh.state.env << std::endl;
  }
  else if (2 == av.size())
  {
//    std::cout << sh.state.env.get(av[1]) << std::endl;
  }
  else if (3 == av.size())
  {
    //sh.state.env.set(av[1], av[2])
  }
  return 0;
}

int cmd_save (shell_t::argv_t const & av, shell_t & sh)
{
  if (av.size() < 2)
  {
    std::cerr << "usage: save handle[+offset] [path]" << std::endl;
    return 1;
  }

  gpi::pc::type::memory_location_t src;
  try
  {
    src = boost::lexical_cast<gpi::pc::type::memory_location_t>(av[1]);
  }
  catch (std::exception const &ex)
  {
    std::cerr << "invalid source: " << av[1]
              << ": " << ex.what()
              << std::endl;
    return -EINVAL;
  }

  gpi::pc::type::handle::descriptor_t d;
  if (sh.state().get_handle_descriptor(src.handle, d) < 0)
  {
    std::cerr << "no such handle: " << src.handle << std::endl;
    return -ESRCH;
  }

  fs::path file_path;
  if (av.size() > 2)
  {
    file_path = av[2];
  }
  else
  {
    file_path = boost::lexical_cast<std::string>(src.handle);
  }

  std::ofstream ofs;
  ofs.open(file_path.string().c_str(), std::ios::binary | std::ios::trunc);
  if (!ofs)
  {
    std::cerr << "could not open file for writing: " << file_path << std::endl;
    return -EIO;
  }

  typedef boost::posix_time::ptime time_type;
  time_type timer_start = boost::posix_time::microsec_clock::local_time();

  gpi::pc::type::memory_location_t shm_com_buf(sh.state().shm_com_hdl(), 0);

  const std::size_t total_to_write =
    (src.offset < d.size) ? d.size - src.offset : 0;

  while (src.offset < d.size)
  {
    print_progress(stderr, src.offset, d.size);

    std::size_t to_write = std::min( sh.state().com_size()
                                   , d.size - src.offset
                                   );

    sh.state ().capi.wait
      (sh.state().capi.memcpy (shm_com_buf, src, to_write, GPI_PC_INVAL));

    ofs.write(sh.state().com_buffer(), to_write);

    src.offset += to_write;
  }

  print_progress(stderr, src.offset, d.size);
  fprintf(stderr, "\n");

  time_type timer_end = boost::posix_time::microsec_clock::local_time();
  double elapsed = (timer_end - timer_start).total_milliseconds () / 1000.0;
  if (elapsed == 0.0)
    elapsed = 1e-15;
  std::cerr << (((double)total_to_write/1024/1024) / elapsed) << " MiB/s"
            << std::endl;

  return 0;
}

int cmd_load (shell_t::argv_t const & av, shell_t & sh)
{
  if (av.size() < 2)
  {
    std::cerr << "usage: load <path> [segment | [handle[+offset]]]" << std::endl;
    return 1;
  }

  const fs::path path (av[1]);
  if (! fs::exists (path))
  {
    std::cerr << "no such file or directory: " << path << std::endl;
    return -EIO;
  }

  std::ifstream ifs;
  ifs.open(path.string().c_str(), std::ios::binary);
  if (!ifs)
  {
    std::cerr << "could not open file for reading: " << path << std::endl;
    return -EIO;
  }

  gpi::pc::type::memory_location_t dst;
  int target_segment = 1;
  if (av.size() > 2)
  {
    try
    {
      dst = boost::lexical_cast<gpi::pc::type::memory_location_t>(av[2]);

      gpi::pc::type::handle::descriptor_t d;
      if (sh.state().get_handle_descriptor(dst.handle, d) < 0)
      {
        std::cerr << "no such handle: " << dst.handle << std::endl;
        return -ESRCH;
      }
    }
    catch (std::exception const &ex1)
    {
      try
      {
        target_segment = boost::lexical_cast<int> (av [2]);
      }
      catch (std::exception const &ex2)
      {
        std::cerr << "invalid destination: '" << av[2] << "'"
                  << " is neither a handle, nor a segment"
                  << std::endl;
        return -EINVAL;
      }
    }
  }

  if (0 == dst.handle)
  {
    std::size_t file_size = fs::file_size(path);

    dst.handle =
      sh.state().capi.alloc( target_segment
                           , file_size
                           , path.string()
                           , gpi::pc::F_GLOBAL
                           | gpi::pc::F_PERSISTENT
                           );
    dst.offset = 0;

    std::cout << dst.handle << std::endl;
  }

  typedef boost::posix_time::ptime time_type;
  time_type timer_start = boost::posix_time::microsec_clock::local_time();
  // read data chunk from file to shm

  std::size_t read_count = 0;
  gpi::pc::type::memory_location_t shm_com_buf(sh.state().shm_com_hdl(), 0);

  gpi::pc::type::handle::descriptor_t handle_descriptor;
  sh.state().get_handle_descriptor(dst.handle, handle_descriptor);

  std::size_t total_to_read = handle_descriptor.size - dst.offset;

  if (total_to_read)
  {
    while (ifs.good() && (read_count < total_to_read))
    {
      print_progress (stderr, read_count, total_to_read);

      std::size_t to_read = std::min( sh.state().com_size()
                                    , total_to_read - read_count
                                    );

      ifs.read(sh.state().com_buffer(), to_read);
      std::size_t read_bytes = ifs.gcount();

      sh.state ().capi.wait
        (sh.state ().capi.memcpy (dst, shm_com_buf, read_bytes, GPI_PC_INVAL));

      read_count += read_bytes;
      dst.offset += read_bytes;
    }

    print_progress (stderr, read_count, total_to_read);
    fprintf(stderr, "\n");

    if (read_count < total_to_read)
    {
      std::cerr << "warning: handle was not completely filled: "
                << "read " << read_count << "/" << total_to_read << " bytes"
                << std::endl;
    }

    time_type timer_end = boost::posix_time::microsec_clock::local_time();
    double elapsed = (timer_end - timer_start).total_milliseconds () / 1000.0;
    if (elapsed == 0.0)
      elapsed = 1e-15;
    std::cerr << (((double)read_count/1024/1024) / elapsed) << " MiB/s"
              << std::endl;
  }

  return 0;
}

int cmd_status (shell_t::argv_t const & av, shell_t & sh)
{
  std::cout << "connected: " << std::boolalpha << sh.state().capi.ping () << std::endl;

  {
    // print internal view of segments
    std::cout << "live segments:" << std::endl;

    gpi::pc::client::api_t::segment_map_t const & m(sh.state().capi.segments());
    BOOST_FOREACH (const gpi::pc::client::api_t::segment_map_t::value_type & seg, m)
    {
      std::cout << *seg.second << std::endl;
    }
  }

  {
    std::cout << "garbage segments:" << std::endl;

    gpi::pc::client::api_t::segment_set_t const & s(sh.state().capi.garbage_segments());
    BOOST_FOREACH (const gpi::pc::client::api_t::segment_set_t::value_type & seg, s)
    {
      std::cout << *seg << std::endl;
    }
  }

  return 0;
}

int cmd_socket (shell_t::argv_t const & av, shell_t & sh)
{
  if (av.size () > 1)
  {
    for ( shell_t::argv_t::const_iterator dir (av.begin()+1)
        ; dir != av.end()
        ; ++dir
        )
    {
      std::cout << "sockets in " << *dir << ":" << std::endl;
      std::cout << collect_sockets (*dir);
      std::cout << std::endl;
      sh.state ().socket_dir = *dir;
    }
  }
  else
  {
    std::cout << "sockets in " << sh.state().socket_dir << ":" << std::endl;
    std::cout << collect_sockets(sh.state().socket_dir);
    std::cout << std::endl;
  }
  return 0;
}

int cmd_segment (shell_t::argv_t const & av, shell_t & sh)
{
  if (av.size() < 2)
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
    shell_t::argv_t new_av (av.begin()+1, av.end());
    int rc (0);

    new_av[0] = "segment-" + av[1];
    rc = sh.execute (new_av);
    if (rc == -1)
    {
      std::cerr << "failed: command not found: " << av[1] << std::endl;
      return 1;
    }
    else
    {
      return rc;
    }
  }
}

int cmd_segment_register (shell_t::argv_t const & av, shell_t & sh)
{
  // usage: create name size [flags]
  std::string name;
  gpi::pc::type::size_t size (0);
  gpi::pc::type::flags_t flags (0);

  if (av.size() < 3)
  {
    std::cerr << "usage: " << "register name size [flags]" << std::endl;
    std::cerr << "    flags:  x - exclusive" << std::endl;
    std::cerr << "            k - keep segment (i.e. no unlink)" << std::endl;
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

    name = av[1];
    size = boost::lexical_cast<gpi::pc::type::size_t>(av[2]);

    if (av.size() > 3)
    {
      std::string flagstring (av[3]);
      for ( std::string::const_iterator f (flagstring.begin())
          ; f != flagstring.end()
          ; ++f
          )
      {
        switch (*f)
        {
        case 'x':
          flags |= gpi::pc::F_EXCLUSIVE;
          break;
        case 'p':
          flags |= gpi::pc::F_PERSISTENT;
          break;
        case 'o':
          flags |= gpi::pc::F_NOCREATE;
          break;
        case 'f':
          flags |= gpi::pc::F_FORCE_UNLINK;
          break;
        default:
          std::cerr << "invalid flag: '" << *f << "'" << std::endl;
          return 2;
        }
      }
    }

    try
    {
      gpi::pc::type::segment_id_t id
        (sh.state().capi.register_segment (name, size, flags));
      std::cout << id << std::endl;
    }
    catch (std::exception const & ex)
    {
      std::cerr << "could not register segment: " << ex.what() << std::endl;
      return 3;
    }
  }

  return 0;
}

int cmd_list (shell_t::argv_t const & av, shell_t & sh)
{
  std::cout << "# *************** #" << std::endl;
  std::cout << "#    Segments     #" << std::endl;
  std::cout << "# *************** #" << std::endl;
  std::cout << std::endl;
  cmd_segment_list (av, sh);
  std::cout << std::endl;

  std::cout << "# *************** #" << std::endl;
  std::cout << "#   Allocations   #" << std::endl;
  std::cout << "# *************** #" << std::endl;
  std::cout << std::endl;
  cmd_memory_list (av, sh);
  std::cout << std::endl;

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
    int mode = 0; // list all
    if (av.size() > 1)
    {
      if (av[1] == "all")
        mode = 0;
      else if (av[1] == "special")
        mode = 1;
      else if (av[1] == "shared")
        mode = 2;
      else if (av[1] == "attached")
        mode = 3;
      else
      {
        std::cerr << "usage: list [all(*) | special | shared | attached]" << std::endl;
        return 1;
      }
    }

    gpi::pc::type::segment::list_t segments
      (sh.state().capi.list_segments());

    std::sort (segments.begin(), segments.end());

    std::cout << gpi::pc::type::segment::ostream_header () << std::endl;
    BOOST_FOREACH (const gpi::pc::type::segment::descriptor_t & desc, segments)
    {
      switch (mode)
      {
      case 0:
        std::cout << desc << std::endl;;
        break;
      case 1:
        if (gpi::flag::is_set (desc.flags, gpi::pc::F_SPECIAL))
        {
          std::cout << desc << std::endl;
        }
        break;
      case 2:
        if (! gpi::flag::is_set (desc.flags, gpi::pc::F_SPECIAL))
        {
          std::cout << desc << std::endl;
        }
        break;
      case 3:
        if (gpi::flag::is_set (desc.flags, gpi::pc::F_ATTACHED))
        {
          std::cout << desc << std::endl;
        }
        break;
      }
    }

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
  int err (0);
  if (av.size() > 1)
  {
    for ( shell_t::argv_t::const_iterator segment(av.begin()+1)
        ; segment != av.end()
        ; ++segment
        )
    {
      try
      {
        gpi::pc::type::segment_id_t seg_id (boost::lexical_cast<gpi::pc::type::segment_id_t>(*segment));
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
  if (av.size() > 1)
  {
    for ( shell_t::argv_t::const_iterator segment(av.begin()+1)
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
  if (av.size() > 1)
  {
    for ( shell_t::argv_t::const_iterator segment(av.begin()+1)
        ; segment != av.end()
        ; ++segment
        )
    {
      try
      {
        gpi::pc::type::segment_id_t seg_id (boost::lexical_cast<gpi::pc::type::segment_id_t>(*segment));
        sh.state().capi.detach_segment (seg_id);
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

int cmd_memory (shell_t::argv_t const & av, shell_t & sh)
{
  if (av.size() < 2)
  {
    std::cout << "usage: memory [command]" << std::endl;
    std::cout << "    alloc" << std::endl;
    std::cout << "    free" << std::endl;
    std::cout << "    copy" << std::endl;
    std::cout << "    wait" << std::endl;
    std::cout << "    list" << std::endl;
    std::cout << "    add" << std::endl;
    std::cout << "    del" << std::endl;
    return 0;
  }
  else
  {
    shell_t::argv_t new_av (av.begin()+1, av.end());
    int rc (0);

    new_av[0] = "memory-" + av[1];
    rc = sh.execute (new_av);
    if (rc == -1)
    {
      std::cerr << "failed: command not found: " << av[1] << std::endl;
      return 1;
    }
    else
    {
      return rc;
    }
  }
}

int cmd_memory_alloc (shell_t::argv_t const & av, shell_t & sh)
{
  if (av.size() < 3)
  {
    std::cout << "usage: alloc size segment [name [flags]]" << std::endl;
    std::cout << "   segment : gpi or id" << std::endl;
    std::cout << "   flags might depend on the segment type:" << std::endl;
    std::cout << "                 p - persistent (not removed after pc death)" << std::endl;
    std::cout << "      gpi flags: g - global (allocate distributed on all node)" << std::endl;
    std::cout << "                 l - local (allocate only on this node)" << std::endl;
    std::cout << "      shm flags: x - exclusive (disallow access from other pc)" << std::endl;
    std::cout << std::endl;
    std::cout << "      global allocs are always persistent" << std::endl;
    std::cout << "      allocs in a non-persistent segment are always temporary" << std::endl;
    std::cout << std::endl;
    return 1;
  }

  gpi::pc::type::segment_id_t seg_id (0);
  gpi::pc::type::size_t size (0);
  std::string desc;
  gpi::pc::type::flags_t flags (gpi::pc::F_GLOBAL);

  size = boost::lexical_cast<size_t>(av[1]);

  if (av[2] == "gpi")
  {
    seg_id = 1;
  }
  else
  {
    seg_id = boost::lexical_cast<gpi::pc::type::segment_id_t> (av[2]);
  }

  if (av.size() > 3)
    desc = av[3];
  else
    desc = "na";

  if (av.size() > 4)
  {
    std::string flagstring(av[4]);
    for ( std::string::const_iterator f (flagstring.begin())
        ; f != flagstring.end()
        ; ++f
        )
    {
      switch (*f)
      {
      case 'l':
      case 'x':
        gpi::flag::unset (flags, gpi::pc::F_GLOBAL);
        break;
      case 'g':
        gpi::flag::set (flags, gpi::pc::F_GLOBAL);
        break;
      case 'p':
        gpi::flag::set (flags, gpi::pc::F_PERSISTENT);
        break;
      default:
          std::cerr << "invalid flag: '" << *f << "'" << std::endl;
          return 2;
        }
    }
  }

  gpi::pc::type::handle_id_t handle
    (sh.state().capi.alloc (seg_id, size, desc, flags));
  if (gpi::pc::type::handle::is_null (handle))
  {
    std::cout << "nil" << std::endl;
    return 1;
  }
  else
  {
    std::cout << gpi::pc::type::handle_t(handle) << std::endl;
    return 0;
  }
}

int cmd_memory_free (shell_t::argv_t const & av, shell_t & sh)
{
  if (av.size() < 2)
  {
    std::cerr << "usage: free handle [handle...]" << std::endl;
    return -1;
  }

  int err (0);
  for ( shell_t::argv_t::const_iterator arg (av.begin()+1)
      ; arg != av.end()
      ; ++arg
      )
  {
    try
    {
      sh.state().capi.free (boost::lexical_cast<gpi::pc::type::handle_t>(*arg));
    }
    catch (std::exception const & ex)
    {
      std::cerr << "free (" << *arg << ") failed: " << ex.what() << std::endl;
      ++err;
    }
  }
  return err;
}

int cmd_memory_copy (shell_t::argv_t const & av, shell_t & sh)
{
  // memcpy <handle>[+offset] handle[+offset] bytes [queue]
  if (av.size() < 4)
  {
    std::cerr << "usage: copy dst[+offset] src[+offset] bytes [queue]" << std::endl;
    std::cerr << "    dst, src : handles" << std::endl;
    std::cerr << "       queue : queue to use (default=0)" << std::endl;
    return 1;
  }

  gpi::pc::type::memory_location_t dst
      (boost::lexical_cast<gpi::pc::type::memory_location_t>(av[1]));
  gpi::pc::type::memory_location_t src
      (boost::lexical_cast<gpi::pc::type::memory_location_t>(av[2]));
  gpi::pc::type::size_t amt
      (boost::lexical_cast<gpi::pc::type::size_t>(av[3]));

  gpi::pc::type::queue_id_t queue = GPI_PC_INVAL;
  if (av.size() > 4)
  {
    queue = boost::lexical_cast<gpi::pc::type::queue_id_t>(av[4]);
  }

  return (int)(sh.state().capi.memcpy (dst, src, amt, queue));
}

int cmd_memory_wait (shell_t::argv_t const & av, shell_t & sh)
{
  if (av.size() > 1)
  {
    std::size_t total (0);
    for (std::size_t i (1); i < av.size(); ++i)
    {
      gpi::pc::type::queue_id_t q(0);

      try
      {
        q = boost::lexical_cast<gpi::pc::type::queue_id_t>(av[i]);
      }
      catch (std::exception const &ex)
      {
        std::cerr << "invalid queue: " << av[i] << " expected non-negative integer!" << std::endl;
        continue;
      }

      gpi::pc::type::size_t done(sh.state().capi.wait(q));
      std::cout << q << " => " << done << std::endl;
      total += done;
    }
    std::cout << "total => " << total << std::endl;
  }
  else
  {
    std::size_t total (0);
    std::vector<gpi::pc::type::size_t> res
      (sh.state().capi.wait ());
    for (std::size_t i(0); i < res.size(); ++i)
    {
      std::cout << i << " => " << res[i] << std::endl;
      total += res[i];
    }
    std::cout << "total => " << total << std::endl;
  }

  return 0;
}

int cmd_memory_list (shell_t::argv_t const & av, shell_t & sh)
{
  // TODO: add sort criteria (created, accessed, name, size, etc.)

  if (!sh.state().capi.is_connected ())
  {
    std::cerr << "not connected to gpi!" << std::endl;
    return 1;
  }

  if (av.size() < 2)
  {
    std::cout << gpi::pc::type::handle::ostream_header() << std::endl;
    gpi::pc::type::handle::list_t handles (sh.state().capi.list_allocations());
    std::sort (handles.begin(), handles.end());
    std::cout << handles;
  }
  else
  {
    std::cout << gpi::pc::type::handle::ostream_header() << std::endl;
    for ( shell_t::argv_t::const_iterator arg(av.begin() + 1)
        ; arg != av.end()
        ; ++arg
        )
    {
      try
      {
        gpi::pc::type::handle::list_t handles
          (sh.state().capi.list_allocations
            (boost::lexical_cast<gpi::pc::type::segment_id_t>(*arg)));
        std::sort (handles.begin(), handles.end());
        std::cout << handles;
      }
      catch (std::exception const & ex)
      {
        std::cerr << "# failed: " << ex.what () << std::endl;
      }
    }
  }
  return 0;
}

int cmd_memory_add (shell_t::argv_t const & av, shell_t & sh)
{
  typedef std::vector<std::string> url_list_t;
  url_list_t urls;

  po::options_description desc ("usage: add [options]");
  desc.add_options ()
    ("help,h", "this help message")
    ( "url,u", po::value<url_list_t>(&urls)
    , "URL of the new memory\npossible parameters: size, mmap, private, persistent"
    )
    ;

  po::positional_options_description pos_opts;
  pos_opts.add ("url", -1);

  po::variables_map vm;
  try
  {
    po::store (po::command_line_parser (av)
              .options (desc)
              .positional (pos_opts)
              .run ()
              , vm
              );
    po::notify (vm);
  }
  catch (std::exception const & ex)
  {
    std::cerr << "invalid argument: " << ex.what() << std::endl;
    std::cerr << "try " << av [0] << " -h to get some help" << std::endl;
    return EXIT_FAILURE;
  }

  if (vm.count ("help"))
  {
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }

  /* FIXME:  this  is a  quite  weird  behavior  of boost::program_options  used
     vectors:

     the original av contains: av[0] -> add av[i] -> params

     when  positional arguments are  used, somehow  av[0] ends  up in  the url
     array..., i.e. we have to drop urls[0]
  */
  if (urls.front () == "add")
    urls.erase (urls.begin ());

  if (urls.empty ())
  {
    std::cerr << "add: url must not be empty" << std::endl;
    std::cerr << desc << std::endl;
    return EXIT_FAILURE;
  }

  int ec = EXIT_SUCCESS;
  BOOST_FOREACH (std::string const &url, urls)
  {
    try
    {
      gpi::pc::type::segment_id_t id =
        sh.state().capi.add_memory (url);
      std::cout << "[" << id << "] = " << url << std::endl;
    }
    catch (std::exception const &ex)
    {
      std::cerr << "add: '" << url << "' failed: " << ex.what ()
                << std::endl;
      ec = EXIT_FAILURE;
    }
  }

  return ec;
}

int cmd_memory_del (shell_t::argv_t const & av, shell_t & sh)
{
  typedef std::vector<std::string> id_list_t;
  id_list_t ids;

  po::options_description desc ("usage: del [options]");
  desc.add_options ()
    ("help,h", "this help message")
    ( "id,i", po::value<id_list_t>(&ids), "the memory ids to remove")
    ;

  po::positional_options_description pos_opts;
  pos_opts.add ("id", -1);

  po::variables_map vm;
  try
  {
    po::store ( po::command_line_parser (av)
              . options (desc)
              . positional (pos_opts)
              . run ()
              , vm
              );
    po::notify (vm);
  }
  catch (std::exception const & ex)
  {
    std::cerr << "invalid argument: " << ex.what() << std::endl;
    std::cerr << "try " << av [0] << " -h to get some help" << std::endl;
    return EXIT_FAILURE;
  }

  if (vm.count ("help"))
  {
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }

  if (ids.front () == "del")
    ids.erase (ids.begin ());

  if (ids.empty ())
  {
    std::cerr << "del: id missing" << std::endl;
    std::cerr << desc << std::endl;
    return EXIT_FAILURE;
  }

  int ec = EXIT_SUCCESS;
  BOOST_FOREACH (std::string const &id_s, ids)
  {
    try
    {
      gpi::pc::type::segment_id_t id =
        boost::lexical_cast<gpi::pc::type::segment_id_t>(id_s);
      sh.state().capi.del_memory (id);
    }
    catch (boost::bad_lexical_cast &)
    {
      std::cerr << "del: '" << id_s << "' failed: not a segment id"
                << std::endl;
      ec = EXIT_FAILURE;
    }
    catch (std::exception const &ex)
    {
      std::cerr << "del: '" << id_s << "' failed: " << ex.what ()
                << std::endl;
      ec = EXIT_FAILURE;
    }
  }

  return ec;
}

path_list_t collect_sockets (fs::path const & prefix)
{
  namespace fs = boost::filesystem;

  std::string file_name_prefix;
  file_name_prefix += prefix.string();
  file_name_prefix += "/S-gpi-space";
  file_name_prefix += ".";
  file_name_prefix += boost::lexical_cast<std::string>(getuid());

  path_list_t paths;
  if (!fs::exists (prefix))
    return paths;

  fs::directory_iterator end_itr;
  for ( fs::directory_iterator itr (prefix)
      ; itr != end_itr
      ; ++itr
      )
  {
    if ( (itr->path().string().find(file_name_prefix) == 0)
       && fs::is_other (itr->status())
       )
    {
      paths.push_back (itr->path());
    }
  }
  return paths;
}

static void print_progress( FILE *fp
                          , const std::size_t current
                          , const std::size_t total
                          , const std::size_t bar_length
                          )
{
  double percent_done = (double)(current) / (double)(total);

  fprintf(stderr, "%3.0f%% [", percent_done*100);
  size_t pos=0;
  for (; total * pos < current * bar_length; ++pos)
  {
    fprintf (stderr, "=");
  }
  for (; pos < bar_length; ++pos)
  {
    fprintf(stderr, " ");
  }
  fprintf(stderr, "]\r");
  fflush(stderr);
}
