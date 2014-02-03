#ifndef SDPA_PLUGIN_SDPACTL_HPP
#define SDPA_PLUGIN_SDPACTL_HPP 1

namespace sdpa
{
  class Control
  {
  public:
    virtual ~Control() {}

    virtual int start () = 0;
    virtual int start (const char *comp) = 0;

    virtual int stop () = 0;
    virtual int stop (const char *comp) = 0;

    virtual int restart () = 0;
    virtual int restart (const char *comp) = 0;

    virtual int status () = 0;
    virtual int status (const char * comp) = 0;
  };
}

#endif
