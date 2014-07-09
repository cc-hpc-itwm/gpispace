// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_RPC_EXCEPTION_SERIALIZATION_HPP
#define FHG_RPC_EXCEPTION_SERIALIZATION_HPP

#include <boost/optional.hpp>

#include <exception>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>

namespace fhg
{
  namespace rpc
  {
    namespace exception
    {
      //! \note needs to either return boost::none; or return a string
      //! which can be used to reconstruct a user_exception.
      //! \note one function may handle multiple exception types.
      using from_exception_ptr_type =
        std::function<boost::optional<std::string> (std::exception_ptr)>;

      //! \note needs to std::throw_with_nested (user_exception (…));
      using throw_with_nested_type = std::function<void (std::string)>;

      //! \note needs to return std::make_exception_ptr (user_exception (…));
      using to_exception_ptr_type =
        std::function<std::exception_ptr (std::string)>;

      //! \note the serialization function which does not return a
      //! boost::none will have the deserialization functions of same
      //! name called. if multiple serialization functions return a
      //! string, it is undefined which one is used, but guaranteed
      //! that deserialization functions of same name are called when
      //! deserializing
      using serialization_functions =
        std::unordered_map<std::string, from_exception_ptr_type>;
      using deserialization_functions =
        std::unordered_map
          <std::string, std::pair<to_exception_ptr_type, throw_with_nested_type>>;

      std::string serialize
        (std::exception_ptr, serialization_functions const&);
      std::exception_ptr deserialize
        (std::string, deserialization_functions const&);
    }
  }
}

#endif
