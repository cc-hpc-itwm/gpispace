#pragma once

#include <boost/filesystem/path.hpp>

namespace xml
{
  namespace parse
  {
    std::string validate_field_name ( const std::string&
                                    , const boost::filesystem::path&
                                    );
  }
}
