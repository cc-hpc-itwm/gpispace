#ifndef SDPA_PLUGIN_SDPACTL_HPP
#define SDPA_PLUGIN_SDPACTL_HPP 1

namespace sdpa
{
  class Control
  {
  public:
    virtual ~Control() {}

    virtual int start () = 0;
    virtual int restart () = 0;
    virtual int stop () = 0;
  };
}

#endif
