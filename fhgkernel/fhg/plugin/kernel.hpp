#ifndef FHG_PLUGIN_KERNEL_HPP
#define FHG_PLUGIN_KERNEL_HPP

namespace fhg
{
  namespace plugin
  {
    class Kernel
    {
    public:
      virtual ~Kernel() {}

      virtual void stop() = 0;
    };
  }
}

#endif
