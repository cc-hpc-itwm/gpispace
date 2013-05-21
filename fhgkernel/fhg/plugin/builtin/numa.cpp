#include <fhglog/minimal.hpp>

#include <hwloc.h>
#include <errno.h>
#include <string.h>

#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/builtin/numa.hpp>

static fhg::numa::Numa *s_global_instance = 0;

namespace fhg
{
  namespace numa
  {
    size_t get_socket ()
    {
      return s_global_instance->socket ();
    }

    unsigned long get_cpuset ()
    {
      return s_global_instance->cpuset ();
    }
  }
}

class NumaImpl : FHG_PLUGIN
               , public fhg::numa::Numa
{
public:
  NumaImpl ()
  {
    hwloc_topology_init (&m_topology);
    hwloc_topology_load (m_topology);
    m_topodepth = hwloc_topology_get_depth (m_topology);

    s_global_instance = this;
  }

  ~NumaImpl ()
  {
    hwloc_topology_destroy (m_topology);

    s_global_instance = 0;
  }

  FHG_PLUGIN_START()
  {
    m_socket = fhg_kernel()->get<size_t>("socket", -1);

    int rc;

    rc = hwloc_get_type_depth (m_topology, HWLOC_OBJ_SOCKET);
    if (rc == HWLOC_TYPE_DEPTH_UNKNOWN)
    {
      MLOG (WARN, "could not get number of sockets");
    }
    else
    {
      size_t nsock = hwloc_get_nbobjs_by_depth (m_topology, rc);

      DMLOG (TRACE, "number of available sockets: " << nsock);

      if (m_socket != (size_t)-1)
      {
        if (m_socket < nsock)
        {
          hwloc_obj_t obj;
          char buf [256];

          obj = hwloc_get_obj_by_type (m_topology, HWLOC_OBJ_SOCKET, m_socket);
          rc = hwloc_set_cpubind ( m_topology
                                 , obj->cpuset
                                 , HWLOC_CPUBIND_PROCESS
                                 );
          m_cpuset = hwloc_bitmap_to_ulong (obj->cpuset);
          hwloc_bitmap_snprintf (buf, sizeof(buf), obj->cpuset);

          if (rc < 0)
          {
            MLOG ( WARN
                 , "could not bind to socket #" << m_socket << " with cpuset " << buf
                 << ": " << strerror (errno)
                 );
          }
          else
          {
            MLOG ( TRACE
                 , "bound to socket #" << m_socket << " with cpuset " << buf
                 );
          }
        }
        else
        {
          MLOG (ERROR, "socket out of range: " << m_socket << "/" << (nsock-1));
          FHG_PLUGIN_FAILED (EINVAL);
        }
      }
    }

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    FHG_PLUGIN_STOPPED();
  }

  size_t socket () const
  {
    return m_socket;
  }

  unsigned long cpuset () const
  {
    return m_cpuset;
  }
private:
  size_t           m_socket;
  hwloc_topology_t m_topology;
  int              m_topodepth;
  unsigned long    m_cpuset;
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
