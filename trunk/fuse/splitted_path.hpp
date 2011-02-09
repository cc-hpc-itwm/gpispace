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

      splitted_path ()
        : segment ()
        , segment_id ()
        , handle ()
        , file ()
      {}
      splitted_path (const std::string & s)
        : segment (s)
        , segment_id ()
        , handle ()
        , file ()
      {}
      splitted_path ( const std::string & s
                    , const alloc::id_t & h
                    )
        : segment (s)
        , segment_id ()
        , handle (h)
        , file ()
      {}
      splitted_path ( const std::string & s
                    , const alloc::id_t & h
                    , const std::string & f
                    )
        : segment (s)
        , segment_id ()
        , handle (h)
        , file (f)
      {}
      splitted_path (const alloc::id_t & h)
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

      void clear_segment () { segment = opt_string (boost::none); }
      void clear_handle () { handle = opt_id (boost::none); }
      void clear_file () { file = opt_string (boost::none); }

      void clear ()
      {
        clear_segment();
        clear_handle();
        clear_file();
      }

      bool operator == (const splitted_path & other) const
      {
        return (this->segment == other.segment)
          && (this->handle == other.handle)
          && (this->file == other.file)
          ;
      }
    };

    std::ostream & operator << (std::ostream & s, const splitted_path sp)
    {
      return
        s << (sp.segment ? *sp.segment : "-")
          << " "
          << (sp.handle ? boost::lexical_cast<std::string>(*sp.handle) : "-")
          << " "
          << (sp.file ? *sp.file : "-");
    }
  } // namespace state
} // namespace gpifs

#endif
