// a very simple config-file manager
// is able to read and write ini-style config files

#include <iostream>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/unordered_map.hpp>

#include <fhg/util/ini-parser.hpp>
#include <fhg/util/ini-parser-helper.hpp>

int main (int ac, char *av[])
{
  namespace po = boost::program_options;

  std::string file_name ("-");

  std::string key;
  std::string val;

  po::options_description desc ("options");
  desc.add_options()
    ("help,h", "print this help")
    ("file,f", po::value<std::string>(&file_name)->default_value(file_name), "file to work on (-: stdin)")
    ("key,k", po::value<std::string>(&key), "key to put or get")
    ("value,v", po::value<std::string>(&val), "value to store")
    ("add,a", "add an entry")
    ("del,d", "delete an entry")
    ("list,l", "list all entries")
    ;

  po::variables_map vm;
  try
  {
    po::store (po::parse_command_line (ac, av, desc), vm);
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

  typedef
    fhg::util::ini::parser::flat_map_parser_t < std::map< std::string
                                                        , std::string
                                                        >
                                              > flat_map_parser_t;

  flat_map_parser_t m;
  try
  {
    if (file_name == "-")
    {
      fhg::util::ini::parse (std::cin, boost::ref(m));
    }
    else
    {
      fhg::util::ini::parse (file_name, boost::ref(m));
    }
  }
  catch (std::exception const & ex)
  {
    std::cerr << "could not parse config: " << std::endl;
    std::cerr << ex.what () << std::endl;
    return EXIT_FAILURE;
  }

  if (vm.count ("list"))
  {
    for ( flat_map_parser_t::entries_t::const_iterator e (m.entries.begin())
        ; e != m.entries.end()
        ; ++e
        )
    {
      std::cout << e->first << " = " << e->second << std::endl;
    }
  }
  else
  {
    std::cout << m.get (key, val) << std::endl;
  }

  return EXIT_SUCCESS;
}
