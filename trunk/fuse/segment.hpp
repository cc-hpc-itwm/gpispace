// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_SEGMENT_HPP
#define FUSE_SEGMENT_HPP 1

#include <string>
#include <sstream>
#include <list>

#include <stdexcept>

#include <id.hpp>
#include <util.hpp>

#include <boost/optional.hpp>
#include <cctype>

namespace gpifs
{
  namespace segment
  {
    typedef id::id_t id_t;
    typedef std::list<id_t> id_list_t;

    // ********************************************************************* //

    STRCONST(root,"/")
    CONST(global)
    CONST(local)
    CONST(shared)
    CONST(proc)

    // ********************************************************************* //

    static inline std::string string (const id_t id)
    {
      switch (id)
        {
        case 0: return global();
        case 1: return local();
        default:
          {
            std::ostringstream str;

            str << shared() << "/" << id;

            return str.str();
          }
        }

      throw std::runtime_error ("segment::string: STRANGE!");
    }

    // ********************************************************************* //

    template<typename IT>
    static inline boost::optional<id_t>
    parse (util::parse::parser<IT> & parser)
    {
      util::parse::skip_space (parser);

      if (parser.end())
        {
          parser.error_set ("expected 'global', 'local' or 'shared'");

          return boost::optional<id_t> (boost::none);
        }
      else
        {
          switch (tolower (*parser))
            {
            case 'g': ++parser;
              return (util::parse::require ("lobal", parser))
                ? boost::optional<id_t> (0)
                : boost::optional<id_t> (boost::none)
                ;

            case 'l': ++parser;
              return (util::parse::require ("ocal", parser))
                ? boost::optional<id_t> (1)
                : boost::optional<id_t> (boost::none)
                ;

            case 's': ++parser;
              return (util::parse::require ("hared", parser))
                ? id::parse (parser)
                : boost::optional<id_t> (boost::none)
                ;

            default:
              parser.error_set ("expected 'global', 'local' or 'shared'");

              return boost::optional<id_t> (boost::none);
            }
        }
    }
  } // namespace segment
} // namespace gpifs

#endif
