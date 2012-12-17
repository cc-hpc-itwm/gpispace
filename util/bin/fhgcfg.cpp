// a very simple config-file manager
// is able to read and write ini-style config files

#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <map>

#include <fhg/util/ini-parser.hpp>
#include <fhg/util/ini-parser-helper.hpp>

int main (int ac, char *av[])
{
  namespace po = boost::program_options;

  std::string file_name;
  std::string key;
  std::string val;

  bool value_was_specified = false;

  po::options_description desc ("options");
  desc.add_options()
    ("help,h", "print this help")
    ("file,f", po::value<std::string>(&file_name), "file to work on (use - for stdin)")
    ("value,v", po::value<std::string>(&val), "value to store")
    ("add,a", po::value<std::string>(&key), "add an entry")
    ("del,d", po::value<std::string>(&key), "delete an entry")
    ("get,g", po::value<std::string>(&key), "get an entry")
    ("list,l",  "list all entries")
    ("print,p", "print ini style format")
    ;
  po::positional_options_description pos_opts;
  pos_opts.add("value", 1);

  po::variables_map vm;
  try
  {
    po::store( po::command_line_parser (ac, av)
             . options(desc).positional(pos_opts).run()
             , vm
             );
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
    std::cerr << "usage: " << av[0] << std::endl;
    std::cerr << std::endl;
    std::cerr << desc << std::endl;
    return EXIT_SUCCESS;
  }

  value_was_specified = vm.count ("value") > 0;

  typedef
    fhg::util::ini::parser::flat_map_parser_t < std::map< std::string
                                                        , std::string
                                                        >
                                              > flat_map_parser_t;

  bool modified (false);
  int exit_code (0);

  flat_map_parser_t m;
  try
  {
    if (file_name == "-")
    {
      std::cerr << "Reading from stdin..." << std::endl;
      fhg::util::ini::parse (std::cin, "<STDIN>", boost::ref(m));
    }
    else
    {
      fhg::util::ini::parse (file_name, boost::ref(m));
    }
  }
  catch (std::exception const &)
  {}

  if (vm.count ("add"))
  {
    if (val.empty())
    {
      std::cerr << "value is empty! use --help to get usage information" << std::endl;
    }
    else
    {
      m.put (key, val);
      modified = true;
    }
  }
  else if (vm.count ("get"))
  {
    if (m.has_key (key) || value_was_specified)
    {
      std::cout << m.get (key, val) << std::endl;
    }
    else
    {
      exit_code = 1;
    }
  }
  else if (vm.count ("del"))
  {
    m.del (key);
    modified = true;
  }
  else if (vm.count ("list"))
  {
    for ( flat_map_parser_t::entries_t::const_iterator e (m.entries.begin())
        ; e != m.entries.end()
        ; ++e
        )
    {
      std::cout << e->first << " = " << e->second << std::endl;
    }
  }
  else if (vm.count("print"))
  {
    m.write (std::cout);
  }

  if (modified)
  {
    try
    {
      if (file_name == "-")
      {
        m.write (std::cout);
      }
      else
      {
        std::ofstream ofs (file_name.c_str());
        if (ofs)
        {
          m.write (ofs);
        }
        else
        {
          throw std::runtime_error ("could not open file: " + file_name);
        }
      }
    }
    catch (std::exception const & ex)
    {
      std::cerr << "could not write config: " << ex.what () << std::endl;
      exit_code = 2;
    }
  }

  return exit_code;
}
