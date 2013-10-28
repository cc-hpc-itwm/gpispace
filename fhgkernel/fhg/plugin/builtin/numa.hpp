#ifndef FHG_PLUGIN_BUILTIN_NUMA_HPP
#define FHG_PLUGIN_BUILTIN_NUMA_HPP 1

namespace fhg
{
  namespace numa
  {
    class Numa
    {
    public:
      virtual ~Numa () {}

      virtual size_t socket () const;
      virtual unsigned long cpuset () const;
    };

    size_t get_socket ();
    unsigned long get_cpuset ();
  }
}

#endif
