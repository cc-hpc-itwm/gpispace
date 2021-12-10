// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <util-generic/warning.hpp>

#include <QtCore/QByteArray>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <sstream>
#include <stdexcept>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      //! \todo directly store into QByteArray instead of extra copy

      struct qbytearray_oarchive
      {
      private:
        std::ostringstream _stream;
        ::boost::archive::binary_oarchive _underlying = {_stream};

      public:
        QByteArray get() const
        {
          std::string const blob_string (_stream.str());
          return QByteArray
            ( blob_string.data()
            , suppress_warning::shorten_64_to_32_with_check<int>
                ( blob_string.size()
                , "serialization too big: QByteArray limited by int"
                )
            );
        }

        using is_saving = decltype (_underlying)::is_saving;
        using is_loading = decltype (_underlying)::is_loading;

        template<typename T> qbytearray_oarchive& operator<< (T const& x)
        {
          return this->operator& (x);
        }
        template<typename T> qbytearray_oarchive& operator& (T const& x)
        {
          _underlying & x;
          return *this;
        }
        template<typename T>
          void save_binary (T* u, size_t count)
        {
          return _underlying.save_binary (u, count);
        }
        template<typename T>
          void register_type (T* u = nullptr)
        {
          return _underlying.register_type (u);
        }
      };

      struct qbytearray_iarchive
      {
      private:
        std::istringstream _stream;
        ::boost::archive::binary_iarchive _underlying = {_stream};

      public:
        qbytearray_iarchive (QByteArray const& ba)
          : _stream ( std::string ( ba.data()
                                  , suppress_warning::sign_conversion<std::size_t>
                                      ( ba.size()
                                      , "qt uses int as size, overflow checked"
                                        " in qbytearray_oarchive"
                                      )
                                  )
                    )
        {}

        using is_saving = decltype (_underlying)::is_saving;
        using is_loading = decltype (_underlying)::is_loading;

        template<typename T> qbytearray_iarchive& operator>> (T& x)
        {
          return this->operator& (x);
        }
        template<typename T> qbytearray_iarchive& operator& (T& x)
        {
          _underlying & x;
          return *this;
        }
        template<typename T>
          void load_binary (T* u, size_t count)
        {
          return _underlying.load_binary (u, count);
        }
        template<typename T>
          void register_type (T* u = nullptr)
        {
          return _underlying.register_type (u);
        }
      };
    }
  }
}
