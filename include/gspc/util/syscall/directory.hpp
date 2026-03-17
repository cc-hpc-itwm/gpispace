#pragma once

#include <filesystem>



    namespace gspc::util::syscall
    {
      struct directory
      {
        directory (std::filesystem::path const&);
        ~directory();

        directory (directory const&) = delete;
        directory& operator= (directory const&) = delete;
        directory (directory&&) = delete;
        directory& operator= (directory&&) = delete;

        int fd() const;
      private:
        /*DIR*/void* _;
      };
    }
