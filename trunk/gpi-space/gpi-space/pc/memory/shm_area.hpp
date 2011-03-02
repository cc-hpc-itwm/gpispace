#ifndef GPI_SPACE_PC_MEMORY_SHM_AREA_HPP
#define GPI_SPACE_PC_MEMORY_SHM_AREA_HPP

#include <gpi-space/pc/memory/memory_area.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class shm_area_t : public virtual area_t
      {
      public:
        static const int area_type = 2;

        shm_area_t ( const gpi::pc::type::id_t id
                   , const gpi::pc::type::process_id_t creator
                   , const std::string & name // == path
                   , const gpi::pc::type::size_t size
                   , const gpi::pc::type::flags_t flags
                   );

        ~shm_area_t ();

        static void* open ( std::string const & path
                          , const gpi::pc::type::size_t size
                          , const int open_flags
                          , const mode_t open_mode = 0
                          );
        static void close (void *, const gpi::pc::type::size_t);
        static void unlink (std::string const &);
      protected:
        bool is_allowed_to_attach (const gpi::pc::type::process_id_t) const;
        grow_direction_t grow_direction (const gpi::pc::type::flags_t) const;
        int get_type_id () const;

        void check_bounds ( const gpi::pc::type::handle::descriptor_t &
                          , const gpi::pc::type::offset_t start
                          , const gpi::pc::type::offset_t end
                          ) const;

        void alloc_hook (const gpi::pc::type::handle::descriptor_t &){}
        void  free_hook (const gpi::pc::type::handle::descriptor_t &){}
      private:
        static bool unlink_after_open (const gpi::pc::type::flags_t);
        static bool unlink_after_close (const gpi::pc::type::flags_t);

        void *m_ptr;
      };
    }
  }
}

#endif // GPI_SPACE_PC_MEMORY_SHM_AREA_HPP
