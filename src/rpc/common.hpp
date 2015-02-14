// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <boost/optional.hpp>

#include <fhg/util/boost/asio/ip/tcp/endpoint.hpp>

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace fhg
{
  namespace rpc
  {
    using endpoint_type = boost::asio::ip::tcp::endpoint;
    using endpoints_type = std::vector<endpoint_type>;

    template<typename T>
      using aggregated_results = std::unordered_map<endpoint_type, T>;

    template<typename T>
      struct aggregated_exception : std::exception
    {
      aggregated_results<T> succeeded;
      aggregated_results<std::exception_ptr> failed;

      aggregated_exception() = default;
      aggregated_exception (decltype (failed) f)
        : failed (std::move (f))
      {}

      virtual const char* what() const noexcept override
      {
        return (std::to_string (failed.size()) + " exceptions").c_str();
      }
    };

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

      struct function_call_result
      {
        uint64_t size_blob;
        bool is_exception;
        char buffer[0];

        static std::size_t required_size (std::string const& blob)
        {
          return sizeof (function_call_result) + blob.size();
        }

        //! \note Assumes to be allocated with underlying storage of
        //! size required_size (blob)
        function_call_result (std::string blob, bool is_exception)
          : size_blob (blob.size())
          , is_exception (is_exception)
        {
          std::copy (blob.begin(), blob.end(), buffer);
        }

        std::string blob() const
        {
          return std::string (buffer, size_blob);
        }
        bool blob_is_exception() const
        {
          return is_exception;
        }
      };
    }
  }
  namespace util
  {
    template<typename Container>
      struct scoped_map_insert
    {
      scoped_map_insert ( Container& container
                        , typename Container::key_type key
                        , typename Container::mapped_type value
                        )
        : _container (container)
        , _key (boost::none)
      {
        if (_container.emplace (key, std::move (value)).second)
        {
          _key = std::move (key);
        }
      }
      scoped_map_insert (scoped_map_insert<Container>&& other)
        : _container (std::move (other._container))
        , _key (boost::none)
      {
        std::swap (_key, other._key);
      }
      ~scoped_map_insert()
      {
        if (_key)
        {
          _container.erase (*_key);
        }
      }

      scoped_map_insert (scoped_map_insert<Container> const&) = delete;
      scoped_map_insert<Container>& operator=
        (scoped_map_insert<Container> const&) = delete;
      scoped_map_insert<Container>& operator=
        (scoped_map_insert<Container>&&) = delete;

      Container& _container;
      boost::optional<typename Container::key_type> _key;
    };

    namespace
    {
      template<typename Container>
        Container& require_no_entry_for_key
          (Container& container, typename Container::key_type const& key)
      {
        if (container.count (key))
        {
          throw std::logic_error ("container already contains key");
        }
        return container;
      }
    }
    template<typename Container>
      struct unique_scoped_map_insert : scoped_map_insert<Container>
    {
      unique_scoped_map_insert ( Container& container
                               , typename Container::key_type key
                               , typename Container::mapped_type value
                               )
        : scoped_map_insert<Container>
          ( require_no_entry_for_key (container, key)
          , std::move (key)
          , std::move (value)
          )
      {}
    };
  }
}
