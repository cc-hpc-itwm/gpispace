#include <fhglog/LogMacros.hpp>

#include <hwloc.h>
#include <errno.h>
#include <string.h>

#include <fhg/plugin/plugin.hpp>

class NumaImpl : FHG_PLUGIN
{
public:
  NumaImpl ()
  {
    hwloc_topology_init (&m_topology);
    hwloc_topology_load (m_topology);
  }

  ~NumaImpl ()
  {
    hwloc_topology_destroy (m_topology);
  }

  FHG_PLUGIN_START()
  {
    const size_t target_socket (fhg_kernel()->get<size_t> ("socket", -1));

    if (target_socket == (size_t)-1)
    {
      FHG_PLUGIN_STARTED();
    }

    const int depth (hwloc_get_type_depth (m_topology, HWLOC_OBJ_SOCKET));
    if (depth == HWLOC_TYPE_DEPTH_UNKNOWN)
    {
      MLOG (ERROR, "could not get number of sockets");
      FHG_PLUGIN_FAILED (EINVAL);
    }

    const size_t available_sockets
      (hwloc_get_nbobjs_by_depth (m_topology, depth));

    DMLOG (TRACE, "number of available sockets: " << available_sockets);

    if (target_socket >= available_sockets)
    {
      MLOG (ERROR, "socket out of range: " << target_socket << "/" << (available_sockets-1));
      FHG_PLUGIN_FAILED (EINVAL);
    }

    const hwloc_obj_t obj
      (hwloc_get_obj_by_type (m_topology, HWLOC_OBJ_SOCKET, target_socket));

    char buf [256];
    hwloc_bitmap_snprintf (buf, sizeof(buf), obj->cpuset);

    if (hwloc_set_cpubind (m_topology, obj->cpuset, HWLOC_CPUBIND_PROCESS) < 0)
    {
      MLOG ( ERROR
           , "could not bind to socket #" << target_socket
           << " with cpuset " << buf << ": " << strerror (errno)
           );
      FHG_PLUGIN_FAILED (EINVAL);
    }


    MLOG (TRACE, "bound to socket #" << target_socket << " with cpuset " << buf);

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    FHG_PLUGIN_STOPPED();
  }

private:
  hwloc_topology_t m_topology;
};

EXPORT_FHG_PLUGIN( numa
                 , NumaImpl
                 , ""
                 , "say hello"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "v0.0.1"
                 , "GPL"
                 , ""
                 , ""
                 );
