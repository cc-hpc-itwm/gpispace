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

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/optional.hpp>

#include <exception>
#include <functional>
#include <string>
#include <unordered_map>

namespace fhg
{
  namespace util
  {
    namespace serialization
    {
      namespace exception
      {
        struct serialization_functions_tuple
        {
          //! \note needs to either return boost::none; or return a
          //! string which can be used to reconstruct a user_exception.
          //! \note one function may handle multiple exception types.
          std::function<boost::optional<std::string> (std::exception_ptr)> from_ptr;
          //! \note needs to return std::make_exception_ptr (user_exception (…));
          std::function<std::exception_ptr (std::string)> to_ptr;
          //! \note needs to std::throw_with_nested (user_exception (…));
          std::function<void (std::string)> throw_with_nested;
        };
        //! \note the serialization function which does not return a
        //! boost::none will have the deserialization functions of
        //! same name called. if multiple serialization functions
        //! return a string, it is undefined which one is used, but
        //! guaranteed that deserialization functions of same name are
        //! called when deserializing
        using serialization_functions =
          std::unordered_map<std::string, serialization_functions_tuple>;

        std::string serialize
          ( std::exception_ptr
          , serialization_functions const& = serialization_functions()
          );
        void serialize
          ( boost::archive::binary_oarchive&
          , std::exception_ptr
          , serialization_functions const& = serialization_functions()
          );

        std::exception_ptr deserialize
          ( std::string
          , serialization_functions const& = serialization_functions()
          );
        std::exception_ptr deserialize
          ( boost::archive::binary_iarchive&
          , serialization_functions const& = serialization_functions()
          );

        //! Make exceptions serialization functions if Exception has either
        //! * boost::serialization support
        //! * Ex::from_string() and ex.to_string()
        //! * Ex::from_exception_ptr(), Ex::to_exception_ptr() and
        //!   Ex::throw_with_nested()
        template<typename Exception>
          serialization_functions::value_type make_functions();
      }
    }
  }
}

#include <util-generic/serialization/exception.ipp>
