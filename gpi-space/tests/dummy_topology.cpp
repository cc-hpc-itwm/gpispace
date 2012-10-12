#include "dummy_topology.hpp"

namespace gpi
{
  namespace tests
  {
    int
    dummy_topology::alloc ( const gpi::pc::type::segment_id_t /* segment */
                          , const gpi::pc::type::handle_t /* handle */
                          , const gpi::pc::type::offset_t /* offset */
                          , const gpi::pc::type::size_t /* size */
                          , const gpi::pc::type::size_t /* local_size */
                          , const std::string & /* name */
                          )
    {
      return 0;
    }

    int dummy_topology::free (const gpi::pc::type::handle_t)
    {
      return 0;
    }


    int dummy_topology::add_memory ( const gpi::pc::type::segment_id_t seg_id
                                   , const std::string & url
                                   )
    {
      return 0;
    }

    int dummy_topology::del_memory (const gpi::pc::type::segment_id_t seg_id)
    {
      return 0;
    }
  }
}
