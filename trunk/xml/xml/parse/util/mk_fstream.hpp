// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_MKFSTREAM_HPP
#define _XML_PARSE_UTIL_MKFSTREAM_HPP 1

#include <fstream>

#include <boost/filesystem.hpp>

#include <xml/parse/state.hpp>
#include <xml/parse/error.hpp>

#include <fhg/util/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      inline std::ofstream & mk_fstream ( std::ofstream & stream
                                        , const state::type & state
                                        , const boost::filesystem::path & file
                                        )
      {
        boost::filesystem::path path (file);

        path.remove_filename();

        if (!fhg::util::mkdirs (path))
          {
            throw error::could_not_create_directory (path);
          }

        if (boost::filesystem::exists (file))
          {
            state.warn (warning::overwrite_file (file));
          }

        stream.open (file.string().c_str());

        if (!stream.good())
          {
            throw error::could_not_open_file (file);
          }

        return stream;
      }
    }
  }
}

#endif
