#ifndef SDPA_CLIENT_CONFIG_HPP
#define SDPA_CLIENT_CONFIG_HPP 1

#include <sdpa/memory.hpp>
#include <sdpa/Properties.hpp>
#include <boost/filesystem.hpp>

namespace sdpa { namespace config {
  class Config {
    public:
      typedef sdpa::shared_ptr<Config> ptr_t;

      static ptr_t create();

      virtual ~Config() {}

      bool is_set(const std::string &key) const { return false; }
      const std::string &get(const std::string &key) const { return key; }

      void parse_env() { }
      void parse_file(const boost::filesystem::path &cfgfile) {}
      void parse(const std::string &cmdline) {}
      void parse(int argc, char **argv) {}
    private:
      Config();
  };

  struct CommonConfiguration {
    Config &operator ()(Config &cfg) {
      return cfg;
    }
  };
}}

#endif
