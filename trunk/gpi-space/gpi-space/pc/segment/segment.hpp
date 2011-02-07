#ifndef GPI_SPACE_PC_SEGMENT_SEGMENT_HPP
#define GPI_SPACE_PC_SEGMENT_SEGMENT_HPP 1

#include <string>
#include <boost/noncopyable.hpp>

#include <gpi-space/pc/type/segment_descriptor.hpp>

namespace gpi
{
  namespace pc
  {
    namespace segment
    {
      class segment_t : boost::noncopyable
      {
      public:
        segment_t ( std::string const & name
                  , type::size_t sz
                  , type::segment_id_t id = type::segment::SEG_INVAL
                  , bool persistent=false
                  );

        ~segment_t ();

        void create ();
        void open ();
        void close ();
        type::segment_id_t id ();

        void assign_id (const type::segment_id_t);

        template<typename T>
        T* ptr () { return (T*)ptr(); }

        void *ptr ();

      private:
        std::string m_name;
        type::segment_id_t m_id;
        type::size_t m_size;
        bool m_persistent;
        void *m_ptr;
        int m_fd;
      };
    }
  }
}

#endif
