// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/temporary_file.hpp>
#include <util-generic/write_file.hpp>

#include <boost/filesystem.hpp>

namespace fhg
{
  namespace util
  {
    class scoped_file_with_content : public temporary_file
    {
    public:
      template<typename T>
      scoped_file_with_content ( ::boost::filesystem::path const& path
                               , T content
                               )
        : temporary_file (path)
      {
        write_file (*this, content);
      }
    };
  }
}
