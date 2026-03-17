#include <gspc/util/syscall/directory.hpp>

#include <gspc/util/syscall.hpp>



    namespace gspc::util::syscall
    {
      directory::directory (std::filesystem::path const& path)
        : _ (syscall::opendir (path.string().c_str()))
      {}
      directory::~directory()
      {
        syscall::closedir (static_cast<DIR*> (_));
      }

      int directory::fd() const
      {
        return syscall::dirfd (static_cast<DIR*> (_));
      }
    }
