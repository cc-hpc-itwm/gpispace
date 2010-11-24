// a very simple config-file manager
// is able to read and write ini-style config files

#include <iostream>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/unordered_map.hpp>

#include <fhg/util/ini-parser.hpp>

struct key_desc_t
{
  std::string sec;
  std::string sec_id;
  bool sec_id_valid;
  std::string key;
};

static void parse_key ( std::string const & full_key, key_desc_t & key )
{
  std::string::size_type sec_pos (full_key.find_first_of ('.'));
  if (sec_pos == std::string::npos)
  {
    throw std::runtime_error ("invalid key: " + full_key);
  }

  key.sec = full_key.substr(0, sec_pos);
  if (key.sec.empty())
  {
    throw std::runtime_error ("empty section: " + full_key);
  }

  std::string::size_type key_pos (full_key.find_last_of ('.'));
  if (key_pos == std::string::npos)
  {
    throw std::runtime_error ("invalid key: " + full_key);
  }

  key.key = full_key.substr (key_pos+1);
  if (key.key.empty())
  {
    throw std::runtime_error ("empty key: " + full_key);
  }

  if (sec_pos != key_pos)
  {
    key.sec_id = full_key.substr (sec_pos+1, key_pos - sec_pos - 1);
    key.sec_id_valid = true;
  }
  else
  {
    key.sec_id_valid = false;
  }
}

static void flatten_key ( key_desc_t const & key, std::string & full_key )
{
  full_key = key.sec;
  if (key.sec_id_valid)
    full_key += "." + key.sec_id;
  full_key += "." + key.key;
}

static std::string flatten_key ( key_desc_t const & key )
{
  std::string k;
  flatten_key(key, k);
  return k;
}

struct flat_map_parser_t
{
  typedef boost::unordered_map<std::string, std::string> entries_t;

  int operator () ( std::string const & sec
                  , std::string const * secid
                  , std::string const & key
                  , std::string const & val
                  )
  {
    return handle (sec,secid,key,val);
  }

  int handle ( std::string const & sec
             , std::string const * secid
             , std::string const & key
             , std::string const & val
             )
  {
    std::string k (secid ? (sec + "." + *secid) : sec);
    k += ".";
    k += key;
    entries[k] = val;
    return 0;
  }

  std::string get ( key_desc_t const & key, std::string const & def )
  {
    try
    {
      return entries.at( flatten_key (key) );
    }
    catch (std::exception const &)
    {
      return def;
    }
  }

  void put ( key_desc_t const & key, std::string const & val )
  {
    entries [flatten_key(key)] = val;
  }

  void del ( key_desc_t const & key )
  {
    entries.erase (flatten_key(key));
  }

  entries_t entries;
};

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
    key_desc_t k;
    parse_key (key, k);

    std::cout << k.sec << std::endl;
    std::cout << k.sec_id << std::endl;
    std::cout << k.sec_id_valid << std::endl;

    std::cout << m.get (k, val) << std::endl;
  }

  return EXIT_SUCCESS;
}
