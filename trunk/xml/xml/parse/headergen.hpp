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

      typedef std::list<descr> descr_list;

      inline void we_header_gen ( const state::type & state
                                , const descr_list & list
                                )
      {
        const boost::filesystem::path prefix (state.path_to_cpp());

        for (descr_list::const_iterator d (list.begin()); d != list.end(); ++d)
          {
            const boost::filesystem::path file (prefix / d->name);

            std::ofstream stream; util::mk_fstream (stream, state, file);

            stream << d->content;

            stream.close();
          }
      }
    }
  }
}

#endif
