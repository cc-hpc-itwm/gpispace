#include <iostream>
#include <fstream>

#include "Config.hpp"

using namespace sdpa::util;
namespace fs = boost::filesystem;

Config::ptr_t Config::create()
{
  Config::ptr_t cfg(new Config());

  return cfg;
}

Config::Config() {

}

/*
Config &Config::load_defaults()
{
  return DefaultConfiguration()(*this);
}
*/

NewConfig::NewConfig(const std::string &component_name, const std::string &env_prefix)
  : component_name_(component_name)
  , env_prefix_(env_prefix)
  , generic_opts_("Generic Options")
  , logging_opts_("Logging configuration")
  , network_opts_("Network Options")
  , tool_opts_("Command-line tool options")
  , tool_hidden_opts_("Command-line tool options (hidden)")
  , specific_opts_(component_name + " specific options")
{
  // fill in defaults
  generic_opts_.add_options()
    ("help,h", "show this help text")
    ("help-module", po::value<std::string>()->implicit_value("help"),
     "show the help for a specific module")
    ("version,V", "print the version number")
    ("dumpversion", "print the version number (short)")
    ("verbose,v", po::value<int>()->implicit_value(1),
     "verbosity level")
    ("quiet,q", "be quiet")
    ;
  logging_opts_.add_options()
    ("logging.file", po::value<std::string>(),
     "redirect log output to this file")
    ("logging.tostderr", "output to stderr")
    ("logging.level", po::value<int>()->default_value(0),
     "standard logging level")
     ;
  network_opts_.add_options()
    ("network.timeout", po::value<unsigned int>()->default_value(1000),
     "maximum time to wait for a reply (in milliseconds)")
    ("network.location", po::value< std::vector<std::string> >()->composing(),
     "location information for a specific location (name:location)")
    ;
}

void NewConfig::parse_command_line(int argc, char **argv)
{
  po::options_description desc;
  desc.add(generic_opts_)
      .add(logging_opts_)
      .add(network_opts_)
      .add(specific_opts_)
      .add(tool_opts_)
      .add(tool_hidden_opts_);

  po::store(po::command_line_parser(argc, argv).options(desc)
                                               .positional(positional_opts_)
                                               .run()
          , opts_);
}
void NewConfig::parse_command_line(const std::vector<std::string> &av)
{
  po::options_description desc;
  desc.add(generic_opts_)
      .add(logging_opts_)
      .add(network_opts_)
      .add(specific_opts_)
      .add(tool_opts_)
      .add(tool_hidden_opts_);
  po::store(po::command_line_parser(av).options(desc)
                                       .positional(positional_opts_)
                                       .run()
          , opts_);
}

void NewConfig::parse_config_file()
{
  if (is_set("config"))
  {
    const std::string &cfg_file = get("config");
	parse_config_file(cfg_file);
  }
}

void NewConfig::parse_config_file(const std::string &cfg_file)
{
  if (is_set("verbose"))
  {
    std::cerr << "I: using config file: " << cfg_file << std::endl;
  }
  std::ifstream cfg_s(cfg_file.c_str());
  parse_config_file(cfg_s, cfg_file);
}

void NewConfig::parse_config_file(std::istream &stream, const std::string &cfg_file)
{
  if (! stream)
  {
    throw std::runtime_error ("could not read config file: " + cfg_file);
  }
  else
  {
	po::options_description desc;
	desc.add(logging_opts_)
	   .add(network_opts_)
	   .add(specific_opts_);
	po::store(po::parse_config_file(stream, desc), opts_);
  }
}

void NewConfig::parse_environment()
{
  parse_environment(env_prefix_);
}
void NewConfig::parse_environment(const std::string &prefix)
{
  po::options_description desc;
  desc.add(logging_opts_)
      .add(network_opts_)
      .add(specific_opts_);
  po::store(po::parse_environment(desc, environment_variable_to_option(prefix)) , opts_);
}

void NewConfig::notify()
{
  po::notify(opts_);
}

std::ostream &NewConfig::printHelp(std::ostream &os) const
{
  po::options_description desc("Allowed Options");
  desc.add(generic_opts_).add(tool_opts_);
  os << desc;
  return os;
}

std::ostream &NewConfig::printModuleHelp(std::ostream &os) const
{
  return printModuleHelp(os, get<std::string>("help-module"));
}

std::ostream &NewConfig::printModuleHelp(std::ostream &os, const std::string &mod) const
{
  if      (mod == "network")       os << network_opts_;
  else if (mod == "logging")       os << logging_opts_;
  else if (mod == component_name_) os << specific_opts_;
  else
  {
    os << "Available modules are:" << std::endl
       << "\t" << "network" << std::endl
       << "\t" << "logging" << std::endl
       << "\t" << component_name_ << std::endl
       << "use --help-module=module to get more information" << std::endl;
  }
  return os;
}
