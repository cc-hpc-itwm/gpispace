#ifndef FHG_PLUGIN_STARTABLE
#define FHG_PLUGIN_STARTABLE 1

namespace fhg
{
  namespace plugin
  {
    class Startable
    {
    public:
      virtual ~Startable() {}
      virtual void start() = 0;
    };
  }
}

#endif
