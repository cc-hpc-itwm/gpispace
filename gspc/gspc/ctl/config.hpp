#ifndef GSPC_CTL_CONFIG_HPP
#define GSPC_CTL_CONFIG_HPP

#include "json_spirit/json_spirit_value.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

namespace gspc
{
  namespace ctl
  {
    namespace error
    {
      enum config_errors
        {
          config_no_section = 1
        , config_no_name
        , config_invalid_key
        , config_invalid
        };
    }

    typedef json_spirit::Value config_t;

    // read the default config
    config_t config_read ();
    config_t config_read_safe (std::string const & file);

    void     config_write (config_t const &, std::string const &fname);

    std::vector<std::string> config_get_all ( config_t const &
                                            , std::string const &key
                                            , std::string const &value_regex=""
                                            );
    std::vector<std::pair<std::string, std::string> > config_list (config_t const &);

    void config_replace ( config_t &
                        , std::string const &key, std::string const& val
                        , std::string const &value_regex=""
                        );
    void config_add ( config_t &
                    , std::string const &key, std::string const& val
                    );
    void config_unset ( config_t &
                      , std::string const &key
                      , std::string const &value_regex=""
                      );
  }
}

#endif
