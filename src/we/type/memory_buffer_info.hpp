#pragma once

#include <boost/serialization/nvp.hpp>

#include <string>

namespace we
{
  namespace type
  {
    struct memory_buffer_info
    {
    public:
      //! \note serialization only
      memory_buffer_info() = default;

      memory_buffer_info
        ( std::string const& size
        , bool const& read_only
        )
        : _size (size)
        , _read_only (read_only)
      {}

      memory_buffer_info (memory_buffer_info const&) = default;
      memory_buffer_info (memory_buffer_info&&) = default;
      memory_buffer_info& operator= (memory_buffer_info const&) = default;
      memory_buffer_info& operator= (memory_buffer_info&&) = default;
      ~memory_buffer_info() = default;

      std::string const& size() const
      {
        return _size;
      }

      bool const& read_only() const
      {
        return _read_only;
      }

    private:
      std::string _size;
      bool _read_only;

      friend class boost::serialization::access;
      template<class Archive>
        void serialize (Archive& ar, const unsigned int)
      {
        ar & _size;
        ar & _read_only;
      }
    };
  }
}
