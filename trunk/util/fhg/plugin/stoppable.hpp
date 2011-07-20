#ifndef FHG_PLUGIN_STOPPABLE
#define FHG_PLUGIN_STOPPABLE 1

namespace fhg
{
  namespace plugin
  {
    class Stoppable
    {
    public:
      virtual ~Stoppable() {}
      virtual void stop() = 0;
    };
  }
}

#endif
