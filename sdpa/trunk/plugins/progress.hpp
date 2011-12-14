#ifndef SDPA_PROGRESS_PLUGIN_HPP
#define SDPA_PROGRESS_PLUGIN_HPP

#include <stddef.h>

namespace progress
{
  class Progress
  {
  public:
    virtual ~Progress() {}

    virtual int set (const char *name, size_t value) = 0;
    virtual int get (const char *name) const = 0;
  };
}

#endif
