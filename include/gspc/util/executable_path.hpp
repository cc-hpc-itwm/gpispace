#pragma once

#include <filesystem>


  namespace gspc::util
  {
    std::filesystem::path executable_path();

    //! Determines executable/library containing the given symbol
    //! rather than the main executable started.
    std::filesystem::path executable_path (void* symbol_in_executable);
    template<typename T>
      std::filesystem::path executable_path (T* symbol_in_executable)
    {
      return executable_path (reinterpret_cast<void*> (symbol_in_executable));
    }
  }
