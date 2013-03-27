// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_HEADER_GEN_HPP
#define _XML_PARSE_HEADER_GEN_HPP 1

#include <boost/filesystem.hpp>

#include <string>
#include <list>

#include <fhg/util/filesystem.hpp>

#include <xml/parse/state.hpp>
#include <xml/parse/util/mk_fstream.hpp>

namespace xml
{
  namespace parse
  {
    namespace includes
    {
      struct descr
      {
        boost::filesystem::path name;
        std::string content;

        descr (const boost::filesystem::path & _n)
          : name (_n)
        {}
      };

      typedef std::list<descr> descrs_type;

      inline void we_header_gen ( const state::type & state
                                , const descrs_type & descrs
                                )
      {
        const boost::filesystem::path prefix (state.path_to_cpp());

        for ( descrs_type::const_iterator d (descrs.begin())
            ; d != descrs.end()
            ; ++d
            )
          {
            util::check_no_change_fstream stream (state, prefix / d->name);

            stream << d->content;
          }
      }
    }
  }
}

#endif
