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

namespace fs = boost::filesystem;

static void long_usage (int lvl);
static void short_usage ();
static fs::path root_path ();
static std::string resolve (fs::path const &p, std::string const &n);

int main (int argc, char *argv [], char *envp [])
{
  int i;
  int verbose = 0;
  int help = 0;
  bool resolve_alias = true;

  const fs::path root (root_path ());
  const fs::path etc_path    (root / "etc");
  const fs::path bin_path    (root / "bin");
  const fs::path lib_path    (root / "lib");
  const fs::path inc_path    (root / "include");
  const fs::path exec_path   (root / "libexec" / "gspc" / "scripts");
  const fs::path plugin_path (root / "libexec" / "fhg" / "plugins");

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
      std::cout << root.string () << std::endl;
      return EX_OK;
    }
    else if (arg ==  "--etc-path")
    {
      ++i;
      std::cout << etc_path.string () << std::endl;
      return EX_OK;
    }
    else if (arg == "--bin-path")
    {
      ++i;
      std::cout << bin_path.string () << std::endl;
      return EX_OK;
    }
    else if (arg == "--lib-path")
    {
      ++i;
      std::cout << lib_path.string () << std::endl;
      return EX_OK;
    }
    else if (arg == "--inc-path")
    {
      ++i;
      std::cout << inc_path.string () << std::endl;
      return EX_OK;
    }
    else if (arg == "--plugin-path")
    {
      ++i;
      std::cout << plugin_path.string () << std::endl;
      return EX_OK;
    }
    else if (arg == "--exec-path")
    {
      ++i;
      std::cout << exec_path.string () << std::endl;
      return EX_OK;
    }
    else
    {
      ++i;
      std::cerr << "gspc: invalid option: " << arg << std::endl;
      return EX_USAGE;
    }
  }

  setenv ("GSPC_HOME", root.string ().c_str (), 1);
  setenv ("GSPC_EXEC_PATH", exec_path.string ().c_str (), 1);
  setenv ( "GSPC_VERBOSE"
         , boost::lexical_cast<std::string>(verbose).c_str ()
         , 1
         );

  std::string subtool;

  if (i < argc)
  {
    subtool = resolve (exec_path, argv [i]);
  }

  if (help)
  {
    if (not subtool.empty ())
    {
      std::string cmd = subtool + " " + "--help";
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

  /*
  ++i;
  std::string cmd = subtool;

  while (i < argc)
  {
    cmd += " \"";
    cmd += argv [i];
    cmd += "\"";
    ++i;
  }
  */

  std::string err, out, inp;

  int rc = gspc::ctl::eval ( subtool, argv + i + 1, argc - (i + 1)
                           , out
                           , err
                           , inp
                           );
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

  return rc;
}

void short_usage ()
{
  std::cerr << "usage: gspc [options] [--] [command [args...]]" << std::endl;
}

void long_usage (int lvl)
{
  std::cerr << "usage: gspc [options] [--] [command [args...]]"     << std::endl
            << ""                                                   << std::endl
            << "   -h|--help      print this help"                  << std::endl
            << "   -v|--verbose   be more verbose"                  << std::endl
            << "   --version      print long version information"   << std::endl
            << "   --dumpversion  print short version information"  << std::endl
            << "   --revision     print revision  information"      << std::endl
            << ""                                                   << std::endl
            << "   --prefix-path  print the installation root"      << std::endl
            << "   --etc-path     print the etc path"               << std::endl
            << "   --bin-path     print the bin path"               << std::endl
            << "   --lib-path     print the lib path"               << std::endl
            << "   --inc-path     print the include path"           << std::endl
            << "   --plugin-path  print the plugin path"            << std::endl
            << "   --exec-path    print the path to gspc tools"     << std::endl
    ;
}

fs::path root_path ()
{
  if (getenv ("GSPC_HOME") != 0)
  {
    return getenv ("GSPC_HOME");
  }
  else
  {
    char buf [4096];
    int rc;

    rc = fhg_get_executable_path (buf, sizeof (buf));
    if (rc < 0)
    {
      std::cerr << "gspc: could not get root path: "
                << "[" << rc << "]: " << strerror (errno)
                << std::endl;
      exit (EX_SOFTWARE);
    }

    return fs::path (buf).parent_path ().parent_path ();
  }
}

std::string resolve (fs::path const &path, std::string const &name)
{
  return (path / ("gspc-" + name)).string ();
}
