// mirko.rahn@itwm.fraunhofer.de

#include <drts/virtual_memory.hpp>
#include <drts/system.hpp>

#include <boost/format.hpp>

#include <exception>

namespace gspc
{
  namespace
  {
    struct popened_file
    {
    public:
      popened_file (const char* command, const char* type)
        : _command (command)
        , _file (popen (command, type))
      {
        if (!_file)
        {
          throw std::runtime_error
            ((boost::format ("Could not popen '%1%'") % _command).str());
        }
      }
      ~popened_file()
      {
        if (pclose (_file) == -1)
        {
          throw std::runtime_error
            ((boost::format ("Could not pclose '%1%'") % _command).str());
        }
      }
      FILE* file() const
      {
        return _file;
      }

    private:
      std::string const _command;
      FILE* _file;
    };

    std::string vmem_alloc ( boost::filesystem::path const& gspc_home
                           , boost::filesystem::path const& vmem_socket
                           , unsigned long const size
                           , std::string const& description
                           )
    {
      std::string const command
        (( boost::format
           ("echo memory-alloc %1% gpi \"%2%\" p | TERM=linux %3% -s %4% | tail -1")
         % size
         % description
         % (gspc_home / "bin" / "gpish")
         % vmem_socket
         ).str()
        );

      popened_file const handle (command.c_str(), "r");

      std::size_t const handle_string_size (18);

      char buf[handle_string_size];

      size_t const read (fread (buf, 1, handle_string_size, handle.file()));

      if (read != handle_string_size)
      {
        int const e (errno);

        throw std::runtime_error
          (( boost::format
             ("Failed to read %5% bytes output of '%1%' (%4%): %2%: %3%")
           % command
           % e
           % strerror (e)
           % read
           % handle_string_size
           ).str()
          );
      }

      return std::string (buf, buf + handle_string_size);
    }
  }

  vmem_allocation::vmem_allocation
    ( scoped_runtime_system const& drts
    , unsigned long size
    , std::string const& description
    )
      : _gspc_home (drts.gspc_home())
      , _vmem_socket (*drts.virtual_memory_socket())
      , _handle (vmem_alloc (_gspc_home, _vmem_socket, size, description))
  {}
  vmem_allocation::~vmem_allocation()
  {
    system (( boost::format ("echo memory-free %1% | %2% -s %3%")
            % _handle
            % (_gspc_home / "bin" / "gpish")
            % _vmem_socket
            ).str()
           , (boost::format ("free %1%") % _handle).str()
           );
  }
}
