#pragma once

#include <vmem/types.hpp>

#include <we/type/range.hpp>

namespace fhg
{
  namespace vmem
  {
    namespace intertwine_compat
    {
      intertwine::vmem::data_id_t data_id (std::string const& serialized)
      {
        std::istringstream iss (serialized);
        boost::archive::binary_iarchive ia (iss);
        intertwine::vmem::data_id_t data_id;
        ia & data_id;
        return data_id;
      }

      intertwine::vmem::global_range_t global_range
        (we::global::range const& range)
      {
        return { data_id (range.handle().name())
               , { intertwine::vmem::offset_t {range.offset()}
                 , intertwine::vmem::size_t {range.size()}
                 }
               };
      }
    }
  }
}
