// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_ALLOC_HPP
#define FUSE_ALLOC_HPP

#include <boost/unordered_set.hpp>
#include <boost/optional.hpp>

#include <list>

#include <id.hpp>
#include <segment.hpp>
#include <util.hpp>

#include <ctime>

namespace gpifs
{
  namespace alloc
  {
    typedef id::id_t id_t;
    typedef std::string name_t;
    typedef std::size_t size_t;

    // ********************************************************************* //

    class descr
    {
    public:
      descr () : _segment (), _size (), _name () {}
      descr ( const segment::id_t & segment
            , const size_t & size
            , const name_t & name
            )
        : _segment (segment)
        , _size (size)
        , _name (name)
      {}

      const segment::id_t & segment () const { return _segment; }
      const size_t & size () const { return _size; }
      const name_t & name () const { return _name; }

    private:
      segment::id_t _segment;
      size_t _size;
      name_t _name;
    };

    static inline std::ostream & operator << (std::ostream & s, const descr & d)
    {
      return
        s << "{" << d.segment() << "/" << d.size() << "/" << d.name() << "}";
    }

    // ********************************************************************* //

    class alloc
    {
    public:
      alloc () : _id (), _ctime (), _descr () {}
      alloc ( const id_t & id
            , const time_t & ctime
            , const segment::id_t & segment
            , const size_t & size
            , const name_t & name
            )
        : _id (id)
        , _ctime (ctime)
        , _descr (segment, size, name)
      {}

      const id_t & id () const { return _id; }
      const time_t & ctime () const { return _ctime; }
      const segment::id_t & segment () const { return _descr.segment(); }
      const size_t & size () const { return _descr.size(); }
      const name_t & name () const { return _descr.name(); }

    private:
      id_t _id;
      time_t _ctime;
      descr _descr;
    };

    typedef std::list<alloc> list_t;
    typedef boost::unordered_set<id_t> id_set_t;

    // ********************************************************************* //

    template<typename IT>
    static inline boost::optional<descr>
    parse (util::parse::parser<IT> & parser)
    {
      const boost::optional<segment::id_t> segment (segment::parse (parser));

      if (segment)
        {
          const boost::optional<size_t> size (id::parse (parser));

          if (size)
            {
              util::parse::skip_space (parser);

              std::string name;

              while (!parser.end() && !isspace (*parser))
                {
                  name.push_back (*parser);
                  ++parser;
                }

              return descr (*segment, *size, name);
            }
        }

      return boost::none;
    }
  } // namespace alloc
} // namespace gpifs

#endif
