#ifndef SDPA_PROGRESS_PLUGIN_HPP
#define SDPA_PROGRESS_PLUGIN_HPP

namespace progress
{
  class Progress
  {
  public:
    virtual ~Progress() {}

    virtual int set (const char *name, int value) = 0;
    virtual int get (const char *name, int *value) const = 0;
  };
}

#endif
