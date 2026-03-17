#pragma once

#include <gspc/util/temporary_file.hpp>
#include <gspc/util/write_file.hpp>

  namespace gspc::util
  {
    class scoped_file_with_content : public temporary_file
    {
    public:
      template<typename T>
      scoped_file_with_content ( std::filesystem::path const& path
                               , T content
                               )
        : temporary_file (path)
      {
        write_file (*this, content);
      }
    };
  }
