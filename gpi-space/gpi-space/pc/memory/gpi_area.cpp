#include "gpi_area.hpp"

#include <utility>

#include <fhglog/minimal.hpp>
#include <gpi-space/gpi/api.hpp>
#include <gpi-space/pc/type/flags.hpp>
#include <gpi-space/pc/global/topology.hpp>

#include <fhg/util/url.hpp>
#include <fhg/util/read_bool.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      gpi_area_t::gpi_area_t ( const gpi::pc::type::process_id_t creator
                             , const std::string & name
                             , const gpi::pc::type::size_t size
                             , const gpi::pc::type::flags_t flags
                             , void * dma_ptr
                             )
        : area_t ( gpi_area_t::area_type
                 , creator
                 , name
                 , size
                 , flags
                 )
        , m_ptr (dma_ptr)
        , m_num_com_buffers (8)
        , m_com_buffer_size (4* (1<<20))
      {
        // total memory size is required for boundary checks
        m_total_memsize = gpi::api::gpi_api_t::get().number_of_nodes () * size;
        m_min_local_offset = gpi::api::gpi_api_t::get().rank() * size;
        m_max_local_offset = m_min_local_offset + size - 1;

        CLOG( DEBUG
            , "gpi.memory"
            , "GPI memory created:"
            <<" per-node: " << size
            <<" total: " << m_total_memsize
            <<" range:"
            <<" ["
            << m_min_local_offset << "," << m_max_local_offset
            << "]"
            );
      }

      void gpi_area_t::init ()
      {
        fhg_assert (m_num_com_buffers > 0);
        fhg_assert (m_com_buffer_size > 0);

        // TODO: make  this lazy, just define  a maximum number  of buffers, but
        // try to allocate them only when actually needed.
        size_t num_buffers_allocated = 0;
        for (size_t i = 0; i < m_num_com_buffers; ++i)
        {
          const std::string hdl_name =
            name () + "-com-" + boost::lexical_cast<std::string>(i);
          try
          {
            gpi::pc::type::handle_t com_hdl =
              this->alloc ( GPI_PC_INVAL
                          , m_com_buffer_size
                          , hdl_name
                          , gpi::pc::F_EXCLUSIVE
                          );
            gpi::pc::type::handle::descriptor_t desc =
              descriptor (com_hdl);

            m_com_handles.add
              (new handle_buffer_t ( com_hdl
                                   , desc.local_size
                                   , (char*)m_ptr + desc.offset
                                   )
              );

            ++num_buffers_allocated;
          }
          catch (std::exception const & ex)
          {
            MLOG (WARN, "could not allocate communication buffer "
                 << (num_buffers_allocated+1)
                 << ": " << ex.what ()
                 );
            break;
          }
        }

        if (0 == num_buffers_allocated)
        {
          throw std::runtime_error
            ( std::string ("no communication buffer could be allocated:")
            + " com-size := " + boost::lexical_cast<std::string>(m_com_buffer_size)
            + " mem-size := " + boost::lexical_cast<std::string>(descriptor ().local_size)
            );
        }
        else
        {
          MLOG ( DEBUG
               , "added " << num_buffers_allocated << "/" << m_num_com_buffers
               << " communication buffers, " << m_com_buffer_size << " bytes each"
               );

          MLOG_IF ( WARN, descriptor ().avail == 0
                  ,  "communication buffers consumed all your precious memory,"
                  << " this might not be what you wanted!"
                  );
        }
      }

      area_t::grow_direction_t
      gpi_area_t::grow_direction (const gpi::pc::type::flags_t flgs) const
      {
        if (gpi::flag::is_set (flgs, gpi::pc::F_GLOBAL))
        {
          return GROW_UP;
        }
        else
        {
          return GROW_DOWN;
        }
      }

      void *
      gpi_area_t::raw_ptr (gpi::pc::type::offset_t off)
      {
        return
          (m_ptr && off < descriptor ().local_size)
          ? ((char*)m_ptr + off)
          : (char*)0;
      }

      bool
      gpi_area_t::is_allowed_to_attach (const gpi::pc::type::process_id_t) const
      {
        return false;
      }

      void
      gpi_area_t::check_bounds ( const gpi::pc::type::handle::descriptor_t &hdl
                               , const gpi::pc::type::offset_t start
                               , const gpi::pc::type::size_t   amount
                               ) const
      {
        if (! (start < hdl.size && (start + amount) <= hdl.size))
        {
          CLOG( ERROR
              , "gpi.memory"
              , "out-of-bounds access:"
              << " hdl=" << hdl
              << " size=" << hdl.size
              << " range=["<<start << ", " << start + amount << "]"
              );
          throw std::invalid_argument
            ("out-of-bounds: access to handle outside boundaries");
        }
      }

      void
      gpi_area_t::alloc_hook (const gpi::pc::type::handle::descriptor_t &hdl)
      {
        if (  gpi::flag::is_set (hdl.flags, gpi::pc::F_GLOBAL)
           && hdl.creator != (gpi::pc::type::process_id_t)(-1)
           )
        {
          gpi::pc::global::topology().alloc ( descriptor ().id
                                            , hdl.id
                                            , hdl.offset
                                            , hdl.size
                                            , hdl.local_size
                                            , hdl.name
                                            );
        }
      }

      void
      gpi_area_t::free_hook (const gpi::pc::type::handle::descriptor_t &hdl)
      {
        if (gpi::flag::is_set (hdl.flags, gpi::pc::F_GLOBAL))
        {
          gpi::pc::global::topology().free(hdl.id);
        }
      }

      bool
      gpi_area_t::is_range_local( const gpi::pc::type::handle::descriptor_t &hdl
                                , const gpi::pc::type::offset_t begin
                                , const gpi::pc::type::offset_t end
                                ) const
      {
        DLOG ( TRACE
             , "checking range: " << "hdl=[" << hdl << "]"
             << " range = [" << begin << ", " << end << ")"
             );

        gpi::pc::type::id_t     my_rank = gpi::api::gpi_api_t::get ().rank ();

        if (not gpi::flag::is_set (hdl.flags, gpi::pc::F_GLOBAL))
          my_rank = 0;

        // my part of the handle is within [my_begin, my_end)
        gpi::pc::type::offset_t my_begin =  my_rank      * hdl.local_size;
        gpi::pc::type::offset_t my_end   = (my_rank + 1) * hdl.local_size;

        if (my_end > hdl.size)
          my_end = hdl.size;

        const bool is_local = my_begin <= begin && end <= my_end;

        return is_local;
      }

      gpi::pc::type::size_t
      gpi_area_t::get_local_size ( const gpi::pc::type::size_t size
                                 , const gpi::pc::type::flags_t flgs
                                 ) const
      {
        if (gpi::flag::is_set (flgs, gpi::pc::F_GLOBAL))
        {
          // static distribution scheme with overhead
          const size_t num_nodes =
            gpi::api::gpi_api_t::get ().number_of_nodes ();
          size_t overhead = (0 != (size % num_nodes)) ? 1 : 0;
          return (size / num_nodes + overhead);
        }
        else
        {
          return size;
        }
      }

      namespace helper
      {
        static
        void do_read_dma_gpi ( const gpi::pc::type::offset_t local_offset
                             , const gpi::pc::type::offset_t remote_offset
                             , const gpi::pc::type::size_t amount
                             , const gpi::pc::type::rank_t from_node
                             , const gpi::pc::type::queue_id_t queue
                             )
        {
          DLOG( TRACE, "read_dma:"
              << " loc-offset = " << local_offset
              << " rem-offset = " << remote_offset
              << " #bytes = " << amount
              << " from = " << from_node
              << " via = " << queue
              );

          gpi::api::gpi_api_t & api = gpi::api::gpi_api_t::get();

          api.read_dma (local_offset, remote_offset, amount, from_node, queue);
        }

        template <typename DMAFun>
        static
        void do_rdma( gpi::pc::type::size_t local_offset
                    , const gpi::pc::type::size_t remote_base
                    , gpi::pc::type::size_t offset
                    , const gpi::pc::type::size_t per_node_size
                    , gpi::pc::type::size_t amount
                    , const gpi::pc::type::queue_id_t queue
                    , DMAFun rdma
                    )
        {
          while (amount)
          {
            const std::size_t rank (offset / per_node_size);
            const std::size_t max_offset_on_rank ((rank + 1) * per_node_size);
            const std::size_t size (std::min ( std::min (per_node_size, amount)
                                             , max_offset_on_rank - offset
                                             )
                                   );
            const std::size_t remote_offset ( remote_base
                                            + (offset % per_node_size)
                                            );

            rdma (local_offset, remote_offset, size, rank, queue);

            local_offset += size;
            offset += size;
            amount -= size;
          }
        }

        static
        void do_read_dma ( const gpi::pc::type::handle::descriptor_t & src_hdl
                         , const gpi::pc::type::size_t src_offset
                         , const gpi::pc::type::handle::descriptor_t & dst_hdl
                         , const gpi::pc::type::size_t dst_offset
                         , const gpi::pc::type::size_t amount
                         , const gpi::pc::type::size_t queue
                         )
        {
          do_rdma( dst_hdl.offset + dst_offset
                 , src_hdl.offset , src_offset
                 , src_hdl.local_size
                 , amount
                 , queue
                 , &do_read_dma_gpi
                 );
        }

        static
        void do_write_dma_gpi ( const gpi::pc::type::offset_t local_offset
                              , const gpi::pc::type::offset_t remote_offset
                              , const gpi::pc::type::size_t amount
                              , const gpi::pc::type::rank_t to_node
                              , const gpi::pc::type::queue_id_t queue
                              )
        {
          DLOG( TRACE, "write_dma:"
              << " loc-offset = " << local_offset
              << " rem-offset = " << remote_offset
              << " #bytes = " << amount
              << " to = " << to_node
              << " via = " << queue
              );

          gpi::api::gpi_api_t & api = gpi::api::gpi_api_t::get();

          api.write_dma (local_offset, remote_offset, amount, to_node, queue);
        }

        static
        void do_write_dma ( const gpi::pc::type::handle::descriptor_t & src_hdl
                          , const gpi::pc::type::size_t src_offset
                          , const gpi::pc::type::handle::descriptor_t & dst_hdl
                          , const gpi::pc::type::size_t dst_offset
                          , const gpi::pc::type::size_t amount
                          , const gpi::pc::type::size_t queue
                          )
        {
          do_rdma( src_hdl.offset + src_offset
                 , dst_hdl.offset , dst_offset
                 , dst_hdl.local_size
                 , amount
                 , queue
                 , &do_write_dma_gpi
                 );
        }

        static
        void do_send ( area_t & src_area
                     , gpi::pc::type::memory_location_t src_loc
                     , gpi_area_t & dst_area
                     , gpi::pc::type::memory_location_t dst_loc
                     , gpi::pc::type::size_t amount
                     , const gpi::pc::type::size_t queue
                     , gpi_area_t::handle_pool_t & handle_pool
                     )
        {
          gpi::api::gpi_api_t & api = gpi::api::gpi_api_t::get();

          handle_buffer_t *buf = handle_pool.acquire ();

          gpi::pc::type::size_t remaining = amount;
          while (remaining)
          {
            buf->used (0);

            api.wait_dma (queue); // make sure previous iteration is finished
            const gpi::pc::type::size_t to_send =
              std::min (remaining, buf->size ());

            const gpi::pc::type::size_t read_bytes =
              src_area.read_from (src_loc, buf->data (), to_send);
            buf->used (read_bytes);

            if (0 == read_bytes)
            {
              MLOG ( WARN
                   , "could not read from src area - premature end-of-file?"
                   );
              break;
            }

            do_write_dma ( dst_area.descriptor (buf->handle ())
                         , 0
                         , dst_area.descriptor (dst_loc.handle)
                         , dst_loc.offset
                         , buf->used ()
                         , queue
                         );

            src_loc.offset += buf->used ();
            dst_loc.offset += buf->used ();
            remaining      -= buf->used ();
          }

          handle_pool.release (buf);
        }

        static
        void do_recv ( area_t & dst_area
                     , gpi::pc::type::memory_location_t dst_loc
                     , gpi_area_t & src_area
                     , gpi::pc::type::memory_location_t src_loc
                     , gpi::pc::type::size_t amount
                     , const gpi::pc::type::size_t queue
                     , gpi_area_t::handle_pool_t & handle_pool
                     )
        {
          gpi::api::gpi_api_t & api = gpi::api::gpi_api_t::get();

          handle_buffer_t *buf = handle_pool.acquire ();

          gpi::pc::type::size_t remaining = amount;
          while (remaining)
          {
            buf->used (0);

            const gpi::pc::type::size_t to_recv =
              std::min (remaining, buf->size ());

            do_read_dma ( src_area.descriptor (src_loc.handle)
                        , src_loc.offset
                        , src_area.descriptor (buf->handle ())
                        , 0
                        , to_recv
                        , queue
                        );
            api.wait_dma (queue);
            buf->used (to_recv);

            const gpi::pc::type::size_t written_bytes =
              dst_area.write_to (dst_loc, buf->data (), buf->used ());

            if (written_bytes != buf->used ())
            {
              MLOG ( WARN
                   , "could not write to dst area - premature end-of-file?"
                   );
              break;
            }

            src_loc.offset += buf->used ();
            dst_loc.offset += buf->used ();
            remaining      -= buf->used ();
          }

          handle_pool.release (buf);
        }
      }

      int
      gpi_area_t::get_specific_transfer_tasks ( const gpi::pc::type::memory_location_t src
                                              , const gpi::pc::type::memory_location_t dst
                                              , area_t & dst_area
                                              , gpi::pc::type::size_t amount
                                              , gpi::pc::type::size_t queue
                                              , task_list_t & tasks
                                              )
      {
        assert (type () == dst_area.type ());

        if (is_local (gpi::pc::type::memory_region_t (src, amount)))
        {
          // write dma
          tasks.push_back
            (boost::make_shared<task_t>
            ( "writeDMA "
            + boost::lexical_cast<std::string> (dst)
            + " <- "
            + boost::lexical_cast<std::string> (src)
            + " "
            + boost::lexical_cast<std::string> (amount)

            , boost::bind ( &helper::do_write_dma
                          , this->descriptor (src.handle)
                          , src.offset
                          , dst_area.descriptor (dst.handle)
                          , dst.offset
                          , amount
                          , queue
                          )
            ));
        }
        else if (dst_area.is_local (gpi::pc::type::memory_region_t (dst, amount)))
        {
          // read dma
          tasks.push_back
            (boost::make_shared<task_t>
            ( "readDMA "
            + boost::lexical_cast<std::string> (dst)
            + " <- "
            + boost::lexical_cast<std::string> (src)
            + " "
            + boost::lexical_cast<std::string> (amount)

            , boost::bind ( &helper::do_read_dma
                          , this->descriptor     (src.handle)
                          , src.offset
                          , dst_area.descriptor (dst.handle)
                          , dst.offset
                          , amount
                          , queue
                          )
            ));
        }
        else
        {
          throw std::runtime_error
            ( "illegal memory transfer requested:"
            " source and destination cannot both be remote!"
            );
        }

        return 0;
      }

      int
      gpi_area_t::get_send_tasks ( area_t & src_area
                                 , const gpi::pc::type::memory_location_t src
                                 , const gpi::pc::type::memory_location_t dst
                                 , gpi::pc::type::size_t amount
                                 , gpi::pc::type::size_t queue
                                 , task_list_t & tasks
                                 )
      {
        tasks.push_back
          (boost::make_shared<task_t>
          ( "send "
          + boost::lexical_cast<std::string> (dst)
          + " <- "
          + boost::lexical_cast<std::string> (src)
          + " "
          + boost::lexical_cast<std::string> (amount)

          , boost::bind ( &helper::do_send
                        , boost::ref (src_area)
                        , src
                        , boost::ref (*this)
                        , dst
                        , amount
                        , queue
                        , boost::ref (m_com_handles)
                        )
          ));
        return 0;
      }

      int
      gpi_area_t::get_recv_tasks ( area_t & dst_area
                                 , const gpi::pc::type::memory_location_t dst
                                 , const gpi::pc::type::memory_location_t src
                                 , gpi::pc::type::size_t amount
                                 , gpi::pc::type::size_t queue
                                 , task_list_t & tasks
                                 )
      {
        tasks.push_back
          (boost::make_shared<task_t>
          ( "recv "
          + boost::lexical_cast<std::string> (dst)
          + " <- "
          + boost::lexical_cast<std::string> (src)
          + " "
          + boost::lexical_cast<std::string> (amount)

          , boost::bind ( &helper::do_recv
                        , boost::ref (dst_area)
                        , dst
                        , boost::ref (*this)
                        , src
                        , amount
                        , queue
                        , boost::ref (m_com_handles)
                        )
          ));
        return 0;
      }

      area_ptr_t gpi_area_t::create (std::string const &url_s)
      {
        using namespace fhg::util;
        using namespace gpi::pc;

        url_t url (url_s);

        type::size_t comsize =
          boost::lexical_cast<type::size_t>(url.get ("buffer_size", "4194304"));
        type::size_t numbuf =
          boost::lexical_cast<type::size_t>(url.get ("buffers", "8"));

        gpi::api::gpi_api_t & gpi_api (gpi::api::gpi_api_t::get());
        gpi_area_t * area = new gpi_area_t ( GPI_PC_INVAL
                                           , "GPI"
                                           , gpi_api.memory_size ()
                                           , gpi::pc::F_PERSISTENT
                                           + gpi::pc::F_GLOBAL
                                           , gpi_api.dma_ptr ()
                                           );
        area->m_num_com_buffers = numbuf;
        area->m_com_buffer_size = comsize;
        return area_ptr_t (area);
      }
    }
  }
}
