#include <gspc/util/executable_path.hpp>
#include <gspc/util/syscall.hpp>

#include <filesystem>


  namespace gspc::util
  {
    std::filesystem::path executable_path()
    {
      //! \todo Other systems. See
      //! http://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe

      return std::filesystem::canonical ( std::filesystem::path ("/")
                                        / "proc"
                                        / std::to_string (syscall::getpid())
                                        / "exe"
                                        );
    }

    std::filesystem::path executable_path (void* symbol_in_executable)
    try
    {
      return std::filesystem::canonical
        (syscall::dladdr (symbol_in_executable).dli_fname);
    }
    catch (...)
    {
      std::throw_with_nested
        (std::runtime_error ("unable to determine executable_path"));
    }
  }
