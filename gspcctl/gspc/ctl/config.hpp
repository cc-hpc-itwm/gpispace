#ifndef GSPC_CTL_CONFIG_HPP
#define GSPC_CTL_CONFIG_HPP

#include <json_spirit_value.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <boost/lexical_cast.hpp>

namespace gspc
{
  namespace ctl
  {
    typedef json_spirit::Value config_t;

    // read the default config
    config_t config_read ();
    config_t config_read (std::istream &is);
    config_t config_read (std::string const & file);

    config_t config_read_site ();
    config_t config_read_system ();
    config_t config_read_user ();

    void     config_write (config_t const &, std::ostream &os);
    void     config_write (config_t const &, std::string const &fname);

    std::string config_get_str (config_t const &, std::string const &key);
    std::vector<std::string> config_get_all (config_t const &, std::string const &key);

    void config_set_str (config_t &, std::string const &key, std::string const& val);
    void config_add_str (config_t &, std::string const &key, std::string const& val);

    template <typename T>
    T config_get (config_t const &cfg, std::string const &key)
    {
      return boost::lexical_cast<T>(config_get_str (cfg, key));
    }

    template <typename T>
    void config_set (config_t &cfg, std::string const &key, T const &val)
    {
      config_set_str (cfg, key, boost::lexical_cast<std::string>(val));
    }

    int config_cmd ( std::vector<std::string> const &argv
                   , std::istream &inp = std::cin
                   , std::ostream &out = std::cout
                   , std::ostream &err = std::cerr
                   );
  }
}

#endif
