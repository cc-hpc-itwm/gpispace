#ifndef GPI_SPACE_TESTS_DUMMY_TOPOLOGY_HPP
#define GPI_SPACE_TESTS_DUMMY_TOPOLOGY_HPP 1

#include <gpi-space/pc/global/itopology.hpp>

namespace gpi
{
  namespace tests
  {
    class dummy_topology : public gpi::pc::global::itopology_t
    {
    public:
      virtual int alloc ( const gpi::pc::type::segment_id_t /* segment */
                , const gpi::pc::type::handle_t /* handle */
                , const gpi::pc::type::offset_t /* offset */
                , const gpi::pc::type::size_t /* size */
                , const gpi::pc::type::size_t /* local_size */
                , const std::string & /* name */
                ) override;

      virtual int free (const gpi::pc::type::handle_t) override;

      virtual int add_memory ( const gpi::pc::type::segment_id_t seg_id
                     , const std::string & url
                     ) override;

      virtual int del_memory (const gpi::pc::type::segment_id_t seg_id) override;

      virtual bool is_master () const override;
    };
  }
}

#endif
