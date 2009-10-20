#ifndef SDPA_CLIENT_CONFIG_HPP
#define SDPA_CLIENT_CONFIG_HPP 1

#include <sdpa/memory.hpp>
#include <sdpa/util/Properties.hpp>
#include <boost/filesystem.hpp>

namespace sdpa { namespace config {
  class Config : public ::sdpa::util::Properties {
    public:
      typedef sdpa::shared_ptr<Config> ptr_t;

      static ptr_t create();

      virtual ~Config() {}

      bool is_set(const std::string &key) const { return has_key(key); }

      Config& load_defaults();
      Config& parse_env() { return *this; }
      Config& parse_file(const boost::filesystem::path & /* cfgfile */) { return *this; }
      Config& parse(const std::string & /* cmdline */) { return *this; }
      Config& parse(int /* argc */, char ** /* argv */) { return *this; }
    private:
      Config();
  };

  struct DefaultConfiguration
  {
    Config &operator()(Config &cfg)
    {
      cfg.put("config.configfile", "/etc/sdpa/sdparc"); // TODO: CMAKE_INSTALL_PREFIX/etc/sdpa/sdparc
      cfg.put("logging.loggers", "default");
      cfg.put("logging.default.level", 0);
      return cfg;
    }
  };

  struct CommonConfiguration
  {
    Config &operator ()(Config &cfg)
    {
      cfg.put("sdpa.logging.level", "error");
      return cfg;
    }
  };
}}

#endif
