#ifndef SDPA_PROGRESS_PLUGIN_HPP
#define SDPA_PROGRESS_PLUGIN_HPP

#include <stddef.h>
#include <string>

namespace progress
{
  class Progress
  {
  public:
    virtual ~Progress() {}

    virtual int set (const std::string &name, size_t value) = 0;
    virtual int initialize (const std::string &name, size_t max) = 0;
    virtual int current (const std::string &name, size_t *value, size_t *max) const = 0;
    virtual int finalize (const std::string &name) = 0;
    virtual int tick (const std::string &name, size_t step) = 0;
  };
}

#endif
