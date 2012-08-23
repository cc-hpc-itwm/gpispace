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
    virtual int reset ( const std::string &name
                      , const std::string &phase
                      , size_t max
                      ) = 0;
    virtual int get ( const std::string &name
                    , size_t *value
                    , size_t *max
                    ) const = 0;
    virtual int finalize (const std::string &name) = 0;
    virtual int tick (const std::string &name, int step = 1) = 0;
  };
}

#endif
