// Copyright (C) 2011-2016,2018-2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <boost/serialization/vector.hpp>

#include <algorithm>
#include <iosfwd>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>


  namespace gspc::we::type
  {
    class GSPC_EXPORT bytearray
    {
    public:
      void push_back (char c);

      bytearray();
      bytearray (std::vector<char> v) : _v (std::move (v)) {}
      bytearray (const char*, std::size_t);
      bytearray (std::string const&);
      std::size_t copy (char* buf, std::size_t size) const;

      template<typename T>
      explicit bytearray (const T* const x)
        : _v ( reinterpret_cast<const char*> (x)
             , reinterpret_cast<const char*> (x) + sizeof (*x)
             )
      {
        static_assert (std::is_trivially_copyable_v<T>);
      }
      template<typename T>
        explicit bytearray
          ( T const& x
          , typename std::enable_if<not std::is_pointer<T>::value>::type* = nullptr
          )
        : _v ( reinterpret_cast<const char*> (&x)
             , reinterpret_cast<const char*> (&x) + sizeof (x)
             )
      {
        static_assert (std::is_trivially_copyable_v<T>);
      }
      template<typename T>
      std::size_t copy (T* const x) const
      {
        static_assert (std::is_trivially_copyable_v<T>);

        return copy (reinterpret_cast<char*> (x), sizeof (*x));
      }

      std::string to_string() const;
      std::vector<char> const& v() const { return _v; }

      GSPC_EXPORT
        friend std::ostream& operator<< (std::ostream&, bytearray const&);
      GSPC_EXPORT
        friend std::size_t hash_value (bytearray const&);
      GSPC_EXPORT
        friend bool operator== (bytearray const&, bytearray const&);
      GSPC_EXPORT
        friend bool operator< (bytearray const&, bytearray const&);

      template<typename T>
      bytearray& operator= (T const& other)
      {
        static_assert (std::is_trivially_copyable_v<T>);

        _v = std::vector<char>
          ( reinterpret_cast<const char*> (&other)
          , reinterpret_cast<const char*> (&other) + sizeof (T)
          );

        return *this;
      }

      template<typename Archive>
        void serialize (Archive& ar, unsigned int)
      {
        ar & _v;
      }

    private:
      std::vector<char> _v;
    };
  }
