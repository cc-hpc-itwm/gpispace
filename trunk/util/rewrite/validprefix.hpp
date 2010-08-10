// mirko.rahn@itwm.fraunhofer.de

#ifndef _REWRITE_VALID_PREFIX_HPP
#define _REWRITE_VALID_PREFIX_HPP 1

#include <boost/filesystem/path.hpp>

#include <fhg/util/starts_with.hpp>

namespace rewrite
{
  static std::string magic_prefix = "_";

  inline bool has_magic_prefix (const std::string & name)
  {
    return fhg::util::starts_with (magic_prefix, name);
  }

  inline std::string mk_prefix (const std::string & name)
  {
    return magic_prefix + name + magic_prefix;
  }
}

#endif
