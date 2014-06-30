// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_RPC_COMMON_HPP
#define FHG_RPC_COMMON_HPP

#include <algorithm>
#include <cstdint>
#include <string>

namespace fhg
{
  namespace rpc
  {
    struct packet_header
    {
      uint64_t message_id;
      uint64_t buffer_size;
      char buffer[0];

      packet_header (uint64_t id, uint64_t size)
        : message_id (id)
        , buffer_size (size)
      {}
    };

    namespace protocol
    {
      struct call_function
      {
        uint64_t size_function_name;
        uint64_t size_arguments;
        char buffer[0];

        static std::size_t required_size
          (std::string const& function_name, std::string const& arguments)
        {
          return sizeof (call_function) + function_name.size() + arguments.size();
        }

        //! \note Assumes to be allocated with underlying storage of
        //! size required_size (function_name, arguments)
        call_function (std::string function_name, std::string arguments)
          : size_function_name (function_name.size())
          , size_arguments (arguments.size())
        {
          std::copy ( function_name.begin(), function_name.end()
                    , buffer
                    );
          std::copy ( arguments.begin(), arguments.end()
                    , buffer + size_function_name
                    );
        }

        std::string function_name() const
        {
          return std::string (buffer, size_function_name);
        }
        std::string arguments() const
        {
          return std::string (buffer + size_function_name, size_arguments);
        }
      };
    }
  }
}

#endif
