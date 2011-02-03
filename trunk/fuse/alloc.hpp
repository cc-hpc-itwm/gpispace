// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_ALLOC_HPP
#define FUSE_ALLOC_HPP

#include <boost/unordered_set.hpp>

#include <list>

#include <id.hpp>
#include <segment.hpp>

#include <ctime>

namespace gpi_fuse
{
  namespace alloc
  {
    typedef id::id_t id_t;
    typedef std::string name_t;
    typedef std::size_t size_t;

    class descr_t
    {
    public:
      descr_t () : _id (), _name (), _size (), _ctime (), _segment () {}
      descr_t ( const id_t & id
              , const name_t & name
              , const size_t & size
              , const time_t & ctime
              , const segment::id_t & segment
              )
        : _id (id)
        , _name (name)
        , _size (size)
        , _ctime (ctime)
        , _segment (segment)
      {}

      const id_t & id () const { return _id; }
      const name_t & name () const { return _name; }
      const size_t & size () const { return _size; }
      const time_t & ctime () const { return _ctime; }
      const segment::id_t & segment () const { return _segment; }

    private:
      id_t _id;
      name_t _name;
      size_t _size;
      time_t _ctime;
      segment::id_t _segment;
    };

    typedef std::list<descr_t> list_t;
    typedef boost::unordered_set<id_t> id_set_t;
  } // namespace alloc
} // namespace gpi_fuse

#endif
