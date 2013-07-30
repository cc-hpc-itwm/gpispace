#include <fhg/util/program_info.h>
#include <fhg/revision.hpp>

#include <errno.h>    // errno
#include <string.h>   // strerror
#include <sysexits.h> // exit codes

#include <string>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

static void long_usage (int lvl);

int main (int argc, char *argv [], char *envp [])
{
  namespace fs = boost::filesystem;
  namespace po = boost::program_options;

  char buf [4096];
  int rc;
  int i;
  int verbose = 0;
  int help = 0;

  rc = fhg_get_executable_path (buf, sizeof (buf));
  if (rc < 0)
  {
    std::cerr << "gspc: could not get my own path: "
              << "[" << rc << "]: " << strerror (errno)
              << std::endl;
    return EX_SOFTWARE;
  }

  const fs::path root (fs::path (buf).parent_path ().parent_path ());
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

  if (help)
  {
    long_usage (help);
    return EX_OK;
  }

  return EX_OK;
}

void long_usage (int lvl)
{
  std::cerr << "usage: gspc [options] [--] [command] [args...]"     << std::endl
            << ""                                                   << std::endl
            << "   -h|--help      print this help"                  << std::endl
            << "   -v|--verbose   be more verbose"                  << std::endl
            << "   --version      print long version information"   << std::endl
            << "   --revision     print revision  information"      << std::endl
            << "   --dumpversion  print short version information"  << std::endl
            << ""                                                   << std::endl
            << "   -v|--verbose   be more verbose"                  << std::endl
            << "   -v|--verbose   be more verbose"                  << std::endl
    ;
}
