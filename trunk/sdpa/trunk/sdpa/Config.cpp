#include "Config.hpp"

using namespace sdpa::config;
namespace fs = boost::filesystem;

Config::ptr_t Config::create()
{
  Config::ptr_t cfg(new Config());

  return cfg;
}

Config::Config() {

}
