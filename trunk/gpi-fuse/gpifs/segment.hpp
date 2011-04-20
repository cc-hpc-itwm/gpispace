// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_SEGMENT_HPP
#define FUSE_SEGMENT_HPP 1

#include <string>
#include <sstream>
#include <list>

#include <stdexcept>

#include <gpifs/id.hpp>
#include <gpifs/util.hpp>

#include <boost/optional.hpp>
#include <cctype>

namespace gpifs
{
  namespace segment
  {
    typedef id::id_t id_t;
    typedef std::list<id_t> id_list_t;

    // ********************************************************************* //

    enum { INVAL  = 0
	 , GLOBAL = 1
	 , LOCAL  = 2
         , GPI = 1
         };

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
	case INVAL: throw std::runtime_error ("segment::string: INVALID");
        case GLOBAL: return global();
        case LOCAL: return local();
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

          return boost::none;
        }
      else
        {
          switch (tolower (*parser))
            {
            case 'g':
              ++parser;

              if (util::parse::require ("lobal", parser))
                {
                  return id_t (GLOBAL);
                }
              else
                {
                  return boost::none;
                }

            case 'l':
              ++parser;

              if (util::parse::require ("ocal", parser))
                {
                  return id_t (LOCAL);
                }
              else
                {
                  return boost::none;
                }

            case 's':
              ++parser;

              if (util::parse::require ("hared", parser))
                {
                  return id::parse (parser);
                }
              else
                {
                  return boost::none;
                }

            default:
              parser.error_set ("expected 'global', 'local' or 'shared'");

              return boost::none;
            }
        }
    }
  } // namespace segment
} // namespace gpifs

#endif
