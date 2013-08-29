#include <fhg/util/program_info.h>
#include <fhg/revision.hpp>

#include <unistd.h>   // exit
#include <errno.h>    // errno
#include <string.h>   // strerror
#include <sysexits.h> // exit codes
#include <stdlib.h>   // system

#include <string>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include <gspc/rif.hpp>
#include <gspc/ctl.hpp>

#include <gspc/ctl/config_cmd.hpp>

namespace fs = boost::filesystem;

static void long_usage (int lvl);
static void short_usage ();
static int resolve ( std::string const &search_path
                   , std::string const &tool
                   , std::vector<std::string> & argv
                   );

int main (int argc, char *argv [], char *envp [])
{
  int i;
  int verbose = 0;
  int help = 0;
  bool resolve_alias = true;

  std::string exec_path = gspc::ctl::exec_path ();

  i = 1;
  while (i < argc)
  {
    std::string arg = argv [i];

    if (arg.size () == 0 || arg [0] != '-')
      break;

    if (arg == "--")
    {
      ++i;
      break;
    }

    if (arg.size () > 1 && arg [0] == '-' && arg [1] != '-')
    {
        ++i;

        std::string::iterator flag = arg.begin ()+1;
        while (flag != arg.end ())
        {
          switch (*flag)
          {
          case 'v':
            ++verbose;
            break;
          case 'h':
            ++help;
            break;
          default:
            std::cerr << "gspc: invalid flag: " << *flag << std::endl;
            return EX_USAGE;
          }
          ++flag;
        }
    }
    else if (arg == "--help")
    {
      ++i;
      ++help;
    }
    else if (arg == "--verbose")
    {
      ++verbose;
      ++i;
    }
    else if (arg == "--version")
    {
      ++i;
      std::cout << "Gpi-Space v" << fhg::project_version () << std::endl;
      return EX_OK;
    }
    else if (arg == "--dumpversion")
    {
      ++i;
      std::cout << fhg::project_version () << std::endl;
      return EX_OK;
    }
    else if (arg == "--revision")
    {
      ++i;
      std::cout << fhg::project_revision () << std::endl;
      return EX_OK;
    }
    else if (arg == "--no-alias")
    {
      ++i;
      resolve_alias = false;
    }
    else if (arg == "--prefix-path")
    {
      ++i;
      std::cout << gspc::ctl::root_path () << std::endl;
      return EX_OK;
    }
    else if (arg ==  "--etc-path")
    {
      ++i;
      std::cout << gspc::ctl::etc_path () << std::endl;
      return EX_OK;
    }
    else if (arg == "--bin-path")
    {
      ++i;
      std::cout << gspc::ctl::bin_path () << std::endl;
      return EX_OK;
    }
    else if (arg == "--lib-path")
    {
      ++i;
      std::cout << gspc::ctl::lib_path () << std::endl;
      return EX_OK;
    }
    else if (arg == "--inc-path")
    {
      ++i;
      std::cout << gspc::ctl::inc_path () << std::endl;
      return EX_OK;
    }
    else if (arg == "--plugin-path")
    {
      ++i;
      std::cout << gspc::ctl::plugin_path () << std::endl;
      return EX_OK;
    }
    else if (arg == "--exec-path")
    {
      ++i;
      std::cout << exec_path << std::endl;
      return EX_OK;
    }
    else if (arg.find ("--exec-path=") == 0)
    {
      ++i;
      std::string par = arg.substr (12);
      if (not par.empty ())
      {
        exec_path = par;
      }
      else
      {
        std::cerr << "gspc: missing parameter to: '--exec-path='" << std::endl;
        return EX_USAGE;
      }
    }
    else
    {
      ++i;
      std::cerr << "gspc: invalid option: " << arg << std::endl;
      return EX_USAGE;
    }
  }

  setenv ("GSPC_HOME", gspc::ctl::root_path ().c_str (), 1);
  setenv ("GSPC_EXEC_PATH", exec_path.c_str (), 1);
  setenv ( "GSPC_VERBOSE"
         , boost::lexical_cast<std::string>(verbose).c_str ()
         , 1
         );

  std::vector<std::string> tool_argv;
  int rc = 0;

  if (i < argc)
  {
    rc = resolve (exec_path, argv [i], tool_argv);
    if (rc < 0)
    {
      std::cerr << "gspc: " << argv [i] << ": " << strerror (errno) << std::endl;
      return EX_USAGE;
    }
  }

  if (help)
  {
    if (not tool_argv.empty ())
    {
      std::string cmd = tool_argv [0] + " " + "--help";
      system (cmd.c_str ());
      return EX_OK;
    }
    else
    {
      long_usage (help);
      return EX_OK;
    }
  }

  if (i == argc)
  {
    short_usage ();
    return EX_USAGE;
  }

  for (int j = i + 1; j < argc ; ++j)
  {
    tool_argv.push_back (argv [j]);
  }

  if (tool_argv [0] == "config")
  {
    rc = gspc::ctl::cmd::config_cmd (tool_argv);
  }
  else
  {
    std::string err, out, inp;

    rc = gspc::ctl::eval (tool_argv, out, err, inp);
    if (rc == 127)
    {
      std::cerr << "gspc: no such command: " << argv [i] << std::endl;
      rc = EX_USAGE;
    }
    else
    {
      std::cout << out;
      std::cerr << err;
    }
  }

  return rc;
}

void short_usage ()
{
  std::cerr << "usage: gspc [options] [--] [command [args...]]" << std::endl;
}

void long_usage (int lvl)
{
  std::cerr
    << "usage: gspc [options] [--] [command [args...]]"             << std::endl
    << ""                                                           << std::endl
    << "   -h|--help              print this help"                  << std::endl
    << "   -v|--verbose           be more verbose"                  << std::endl
    << "   --version              print long version information"   << std::endl
    << "   --dumpversion          print short version information"  << std::endl
    << "   --revision             print revision  information"      << std::endl
    << ""                                                           << std::endl
    << "   --prefix-path          print the installation root"      << std::endl
    << "   --etc-path             print the etc path"               << std::endl
    << "   --bin-path             print the bin path"               << std::endl
    << "   --lib-path             print the lib path"               << std::endl
    << "   --inc-path             print the include path"           << std::endl
    << "   --plugin-path          print the plugin path"            << std::endl
    << "   --exec-path[=path]     print/set the path to gspc tools" << std::endl
    ;
}

int resolve ( std::string const &path
            , std::string const &name
            , std::vector<std::string> & argv
            )
{
  // resolve alias here
  std::string unaliased_command = name;

  if (unaliased_command == "config")
  {
    argv.push_back ("config");
    return 1;
  }
  else
  {
    fs::path tool = fs::path (path) / ("gspc-" + unaliased_command);

    if (fs::exists (tool))
    {
      argv.push_back (tool.string ());
      return 0;
    }
    else
    {
      errno = ENOENT;
      return -1;
    }
  }
}
