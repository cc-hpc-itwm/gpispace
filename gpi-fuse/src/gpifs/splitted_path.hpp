// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_SPLITTED_PATH_HPP
#define FUSE_SPLITTED_PATH_HPP 1

#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>

#include <string>

#include <iostream>

namespace gpifs
{
  namespace state
  {
    struct splitted_path
    {
    public:
      typedef boost::optional<std::string> opt_string;
      typedef boost::optional<alloc::id_t> opt_id;

      opt_string segment;
      opt_id segment_id;
      opt_id handle;
      opt_string file;

      splitted_path ( const std::string & s
                    , const segment::id_t & sid
                    , const alloc::id_t & h
                    , const std::string & f
                    )
        : segment (s)
        , segment_id (sid)
        , handle (h)
        , file (f)
      {}
      splitted_path ( const std::string & s
                    )
        : segment (s)
        , segment_id ()
        , handle ()
        , file ()
      {}
      splitted_path ( const std::string & s
                    , const segment::id_t & sid
                    , const alloc::id_t & h
                    )
        : segment (s)
        , segment_id (sid)
        , handle (h)
        , file ()
      {}
      splitted_path ( const alloc::id_t & h
                    )
        : segment ()
        , segment_id ()
        , handle (h)
        , file ()
      {}
      splitted_path ( const alloc::id_t & h
                    , const std::string & f
                    )
        : segment ()
        , segment_id ()
        , handle (h)
        , file (f)
      {}
      splitted_path ( const std::string & s
                    , const segment::id_t & sid
                    )
        : segment (s)
        , segment_id (sid)
        , handle ()
        , file ()
      {}
      splitted_path ( const std::string & s
                    , const std::string & f
                    )
        : segment (s)
        , segment_id ()
        , handle ()
        , file (f)
      {}

      bool operator == (const splitted_path & other) const
      {
        return (this->segment == other.segment)
          && (this->handle == other.handle)
          && (this->file == other.file)
          ;
      }
    };

    typedef boost::optional<splitted_path> maybe_splitted_path;

    std::ostream & operator << (std::ostream & s, const splitted_path sp)
    {
      return
        s << (sp.segment ? *sp.segment : "-")
          << " "
          << (sp.segment_id ? boost::lexical_cast<std::string>(*sp.segment_id) : "-")
          << " "
          << (sp.handle ? boost::lexical_cast<std::string>(*sp.handle) : "-")
          << " "
          << (sp.file ? *sp.file : "-");
    }
  } // namespace state
} // namespace gpifs

#endif
