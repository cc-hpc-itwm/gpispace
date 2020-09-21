#pragma once

#include <string>
#include <boost/noncopyable.hpp>

#include <iml/vmem/gaspi/pc/type/types.hpp>

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
                  );

        ~segment_t ();

        void create (const mode_t mode = 00600);
        void open ();
        void close ();
        void unlink ();

        void assign_id (const type::segment_id_t);

        template<typename T>
        T* ptr () { return (T*)ptr(); }

        void *ptr ();
        const void *ptr () const;
      private:
        void *m_ptr;
        std::string _name;
        std::size_t _size;
      };
    }
  }
}
