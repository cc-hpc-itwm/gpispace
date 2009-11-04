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

Config &Config::load_defaults()
{
  return DefaultConfiguration()(*this);
}
