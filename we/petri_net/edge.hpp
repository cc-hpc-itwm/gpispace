// mirko.rahn@itwm.fraunhofer.de

#ifndef _PETRI_NET_EDGE_HPP_5ab461a6_23a3_4155_998b_edcf5aac0bff
#define _PETRI_NET_EDGE_HPP_5ab461a6_23a3_4155_998b_edcf5aac0bff

namespace petri_net
{
  namespace edge
  {
    enum type {PT,PT_READ,TP};

    static inline bool is_pt_read (const type & e)
    {
      return e == PT_READ;
    }

    static inline bool is_PT (const type & et)
    {
      return (et == PT || et == PT_READ);
    }

    static inline type pt_read (void)
    {
      return PT_READ;
    }
  }
} // namespace petri_net

#endif // _PETRI_NET_EDGE_HPP_5ab461a6_23a3_4155_998b_edcf5aac0bff
