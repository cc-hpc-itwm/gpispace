#pragma once

#include <string>
#include <boost/noncopyable.hpp>

#include <iml/vmem/gaspi/pc/type/segment_descriptor.hpp>

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
                  , const type::size_t sz
                  , const type::segment_id_t id = type::segment::SEG_INVAL
                  );

        ~segment_t ();

        void create (const mode_t mode = 00600);
        void open ();
        void close ();
        void unlink ();

        void assign_id (const type::segment_id_t);
        type::segment_id_t id () const { return m_descriptor.id; }

        type::segment::descriptor_t const & descriptor() const { return m_descriptor; }
        type::segment::descriptor_t & descriptor() { return m_descriptor; }

        template<typename T>
        T* ptr () { return (T*)ptr(); }

        void *ptr ();
        const void *ptr () const;
      private:
        gpi::pc::type::segment::descriptor_t m_descriptor;
        void *m_ptr;
        std::string _name;
        std::size_t _size;
      };
    }
  }
}
