// a very simple config-file manager
// is able to read and write ini-style config files

#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <map>

#include <fhg/util/ini-parser.hpp>

#include <fhg/util/split.hpp>
#include <fhg/util/join.hpp>

#include <boost/foreach.hpp>

namespace
{
  void write (std::ostream& os, std::map<std::string, std::string> const& m)
  {
    boost::optional<std::string> section_label;
    std::string section_sublabel;

    BOOST_FOREACH
      (std::pair<std::string BOOST_PP_COMMA() std::string> const& kv, m)
    {
      std::list<std::string> path
        (fhg::util::split< std::string
                         , std::list<std::string>
                         > (kv.first, '.')
        );

      if (path.empty())
      {
        throw std::runtime_error ("write: empty key");
      }

      std::string const key (path.back()); path.pop_back();

      if (path.empty() && !section_label)
      {
        throw std::runtime_error ("write: initial section without label");
      }

      std::string const label (path.front()); path.pop_front();
      std::string const sublabel (fhg::util::join (path, "."));

      if (label != section_label || sublabel != section_sublabel)
      {
        if (section_label)
        {
          os << "\n";
        }

        section_label = label;
        section_sublabel = sublabel;

        os << "[" << *section_label;
        if (!section_sublabel.empty())
        {
          os << " \"" << section_sublabel << "\"";
        }
        os << "]\n";
      }

      os << "  " << key << " = " << kv.second << "\n";
    }
  }
}

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
    ("get-regex", po::value<std::string>(&key), "get entries by regex")
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

  bool modified (false);
  int exit_code (0);

  fhg::util::ini m (file_name);

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
  else if (vm.count ("get-regex"))
  {
    boost::regex ex (key);
    exit_code = 1;
    BOOST_FOREACH
      ( std::pair<std::string BOOST_PP_COMMA() std::string> const& kv
      , m.assignments()
      )
    {
      if (boost::regex_search (kv.first, ex))
      {
        std::cout << kv.first << " = " << kv.second << std::endl;
        exit_code = 0;
      }
    }
  }
  else if (vm.count ("get"))
  {
    if (m.get (key) || value_was_specified)
    {
      std::cout << m.get (key).get_value_or (val) << std::endl;
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
    BOOST_FOREACH
      ( std::pair<std::string BOOST_PP_COMMA() std::string> const& kv
      , m.assignments()
      )
    {
      std::cout << kv.first << " = " << kv.second << std::endl;
    }
  }
  else if (vm.count("print"))
  {
    write (std::cout, m.assignments());
  }

  if (modified)
  {
    try
    {
      if (file_name == "-")
      {
        write (std::cout, m.assignments());
      }
      else
      {
        std::ofstream ofs (file_name.c_str());
        if (ofs)
        {
          write (ofs, m.assignments());
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
