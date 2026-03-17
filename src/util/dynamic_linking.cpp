#include <gspc/util/dynamic_linking.hpp>

#include <utility>

#include <link.h>


  namespace gspc::util
  {
    scoped_dlhandle::scoped_dlhandle
      ( std::filesystem::path const& path
      , int flags
      )
        : _ {syscall::dlopen (path.c_str(), flags)}
    {}

    scoped_dlhandle::~scoped_dlhandle()
    {
      if (_)
      {
        syscall::dlclose (_);
      }
    }

    scoped_dlhandle::scoped_dlhandle (scoped_dlhandle&& other) noexcept
      : _ (std::move (other._))
    {
      other._ = nullptr;
    }

    std::vector<std::filesystem::path> currently_loaded_libraries()
    {
      std::vector<std::filesystem::path> result;
      dl_iterate_phdr
        ( +[] (dl_phdr_info* info, size_t, void* data)
           {
             static_cast<std::vector<std::filesystem::path>*> (data)
               ->emplace_back (info->dlpi_name);
             return 0;
           }
        , &result
        );
      return result;
    }
  }
