#include "memory_transfer_t.hpp"

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      // static const char *status_name[] =
      // {
      //   "PENDING"
      // , "RUNNING"
      // , "FAILED"
      // , "FINISHED"
      // };

      std::ostream & operator << (std::ostream &os, const memory_transfer_t &mt)
      {
        //        os << status_name[mt.status] << " "
        os << mt.amount << " bytes "
           << mt.src_location << " --> " << mt.dst_location
           << " via "
           << mt.queue
           ;
        return os;
      }
    }
  }
}
