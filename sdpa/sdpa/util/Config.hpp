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

  class Config : public boost::property_tree::ptree {
  };

// new config

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
