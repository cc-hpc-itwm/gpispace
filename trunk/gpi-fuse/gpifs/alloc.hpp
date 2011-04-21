// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_ALLOC_HPP
#define FUSE_ALLOC_HPP

#include <ctime>

#include <list>

#include <boost/unordered_set.hpp>
#include <boost/optional.hpp>

#include <gpifs/id.hpp>
#include <gpifs/segment.hpp>
#include <gpifs/util.hpp>

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
            , const bool global
            )
        : _segment (segment)
        , _size (size)
        , _name (name)
        , _global (global)
      {}

      const segment::id_t & segment () const { return _segment; }
      const size_t & size () const { return _size; }
      const name_t & name () const { return _name; }
      const bool global () const { return _global; }

    private:
      segment::id_t _segment;
      size_t _size;
      name_t _name;
      bool _global;
    };

    static inline std::ostream & operator << (std::ostream & s, const descr & d)
    {
      return
        s << "{"
          << d.global()
          << "/"
          << d.segment()
          << "/"
          << d.size()
          << "/"
          << d.name()
          << "}";
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
            , const bool global
            )
        : _id (id)
        , _ctime (ctime)
        , _descr (segment, size, name, global)
      {}

      const id_t & id () const { return _id; }
      const time_t & ctime () const { return _ctime; }
      const segment::id_t & segment () const { return _descr.segment(); }
      const size_t & size () const { return _descr.size(); }
      const name_t & name () const { return _descr.name(); }
      const bool global() const { return _descr.global(); }

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

              if (*segment == segment::GLOBAL)
              {
                return descr (segment::GPI, *size, name, true);
              }
              else if (*segment == segment::LOCAL)
              {
                return descr (segment::GPI, *size, name, false);
              }
            }
        }

      return boost::none;
    }
  } // namespace alloc
} // namespace gpifs

#endif
