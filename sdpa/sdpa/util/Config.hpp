#ifndef SDPA_CLIENT_CONFIG_HPP
#define SDPA_CLIENT_CONFIG_HPP 1

#include <sdpa/memory.hpp>
#include <boost/filesystem.hpp>

#include <boost/program_options.hpp>
#include <algorithm> // std::transform
#include <cctype> // std::tolower

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <sdpa/logging.hpp>
#include <sdpa/SDPAException.hpp>

namespace sdpa { namespace util {

 class InvalidConfiguration  : public sdpa::SDPAException
 {
		public:
		 InvalidConfiguration(const std::string& reason): sdpa::SDPAException(reason) {}

		virtual ~InvalidConfiguration() throw() {}
 };

  class Config : public boost::property_tree::ptree {
    public:
      typedef sdpa::shared_ptr<Config> ptr_t;

      static ptr_t create();

      virtual ~Config() {}

      bool is_set(const std::string &key) const
      {
    	  boost::property_tree::ptree::const_assoc_iterator it(find(key));
    	  return ( it != not_found() );
      }

      void read(std::string inFile ) throw (InvalidConfiguration)
      {
    	  try {
    		  read_ini(inFile, *dynamic_cast<boost::property_tree::ptree*>(this));
    	  }
    	  catch ( const boost::property_tree::ini_parser_error& ex )
    	  {
    		  throw InvalidConfiguration(std::string("Invalid configuration: ") + ex.what());
    	  }
      }

      void write(std::ostream& os) const
      {
    	  write_ini(os, *this);
      }

      void print()
      {
    	  std::ostringstream sstr;
    	  write(sstr);
    	  DMLOG(TRACE, "The daemon was configured with the following parameters: \n\n" << sstr.str());
      }

    //private:
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

// new config

  struct environment_variable_to_option
  {
  public:
    explicit
    environment_variable_to_option(const std::string prefix)
      : p(prefix) {}

    std::string operator()(const std::string &var)
    {
      if (var.substr(0, p.size()) == p)
      {
        std::string option = var.substr(p.size());
        std::transform(option.begin(), option.end(), option.begin(), tolower);
        for (std::string::iterator c(option.begin()); c != option.end(); ++c)
        {
          if (*c == '_') *c = '.';
        }
        return option;
      }
      return "";
    }

  private:
    std::string p;
  };

  namespace po = boost::program_options;

  class NewConfig
  {
  public:
    NewConfig(const std::string &component_name, const std::string &env_prefix);

    void parse_command_line(int argc, char **argv);
    void parse_command_line(const std::vector<std::string> &);

    void parse_config_file();
    void parse_config_file(const std::string &cfg_file_path);
    void parse_config_file(std::istream &stream, const std::string &cfg_file);

    void parse_environment();
    void parse_environment(const std::string &prefix);

    void notify(); // notify that parsing is finished

    po::options_description &specific_opts() { return specific_opts_; }
    po::options_description &tool_opts() { return tool_opts_; }
    po::options_description &tool_hidden_opts() { return tool_hidden_opts_; }
    po::positional_options_description &positional_opts() { return positional_opts_; }
    po::options_description& network_opts() { return network_opts_;} // network related options

    std::ostream &printHelp(std::ostream &) const;
    std::ostream &printModuleHelp(std::ostream &os) const;
    std::ostream &printModuleHelp(std::ostream &os, const std::string &mod) const;

    const std::string &get(const std::string &name) const { return opts_[name].as<std::string>(); }
    template <typename T> const T &get(const std::string &name) const { return opts_[name].as<T>(); }
    template <typename T> const T &operator[](const std::string &name) const { return get<T>(name); }

    bool is_set(const std::string &name) const { return (opts_.count(name) > 0); }
  private:
    std::string component_name_;
    std::string env_prefix_;

    po::options_description generic_opts_; // supported by all command line tools
    po::options_description logging_opts_; // logging related options
    po::options_description network_opts_; // network related options
    po::options_description tool_opts_;    // additional command line options
    po::options_description tool_hidden_opts_; // hidden command line options
    po::options_description specific_opts_; // specific options to a component, feel free to modify them
    po::positional_options_description positional_opts_; // positional parameters used for command line parsing

    po::variables_map opts_;
  };
}}

#endif
