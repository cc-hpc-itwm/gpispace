#pragma once

#include <filesystem>
#include <string>
#include <vector>


  namespace gspc::util
  {
    std::vector<std::string> read_lines (std::filesystem::path const&);
  }
