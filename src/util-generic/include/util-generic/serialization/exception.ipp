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

#include <util-generic/cxx17/void_t.hpp>

#include <sstream>

namespace fhg
{
  namespace util
  {
    namespace serialization
    {
      namespace exception
      {
        namespace detail
        {
          template<typename Exception, typename = void>
          struct serializer;

          template<typename Exception>
            struct serializer
              < Exception
              , cxx17::void_t
                  < decltype (std::declval<::boost::archive::binary_oarchive>() << std::declval<Exception const&>())
                  , decltype (std::declval<::boost::archive::binary_iarchive>() >> std::declval<Exception&>())
                  , decltype (std::declval<Exception&>() = std::declval<Exception>())
                  , decltype (Exception{})
                  >
              >
          {
            static ::boost::optional<std::string> from_ptr (std::exception_ptr ex_ptr)
            {
              try
              {
                std::rethrow_exception (ex_ptr);
              }
              catch (Exception const& ex)
              {
                std::ostringstream oss;
                ::boost::archive::binary_oarchive oa (oss);
                oa << ex;
                return oss.str();
              }
              catch (...) {}

              return ::boost::none;
            }

            static Exception from_string (std::string const& serialized)
            {
              std::istringstream iss (serialized);
              ::boost::archive::binary_iarchive ia (iss);
              Exception ex;
              ia >> ex;
              return ex;
            }

            static std::exception_ptr to_ptr (std::string const& serialized)
            {
              return std::make_exception_ptr (from_string (serialized));
            }

            static void throw_with_nested (std::string const& serialized)
            {
              std::throw_with_nested (from_string (serialized));
            }
          };

          template<typename Exception>
            struct serializer
              < Exception
              , cxx17::void_t
                  < decltype (std::declval<Exception const&>().serialize (std::declval<::boost::archive::binary_oarchive&>()))
                  , decltype (std::declval<Exception&>() = Exception::deserialize (std::declval<::boost::archive::binary_iarchive&>()))
                  >
              >
          {
            static ::boost::optional<std::string> from_ptr (std::exception_ptr ex_ptr)
            {
              try
              {
                std::rethrow_exception (ex_ptr);
              }
              catch (Exception const& ex)
              {
                std::ostringstream oss;
                ::boost::archive::binary_oarchive oa (oss);
                ex.serialize (oa);
                return oss.str();
              }
              catch (...) {}

              return ::boost::none;
            }

            static Exception from_string (std::string const& serialized)
            {
              std::istringstream iss (serialized);
              ::boost::archive::binary_iarchive ia (iss);
              return Exception::deserialize (ia);
            }

            static std::exception_ptr to_ptr (std::string const& serialized)
            {
              return std::make_exception_ptr (from_string (serialized));
            }

            static void throw_with_nested (std::string const& serialized)
            {
              std::throw_with_nested (from_string (serialized));
            }
          };

          template<typename Exception>
            struct serializer
              < Exception
              , cxx17::void_t
                  < decltype (std::declval<std::string&>() = std::declval<Exception const&>().to_string())
                  , decltype (Exception::from_string (std::declval<std::string>()))
                  >
              >
          {
            static ::boost::optional<std::string> from_ptr (std::exception_ptr ex_ptr)
            {
              try
              {
                std::rethrow_exception (ex_ptr);
              }
              catch (Exception const& ex)
              {
                return ex.to_string();
              }
              catch (...) {}

              return ::boost::none;
            }

            static std::exception_ptr to_ptr (std::string const& serialized)
            {
              return std::make_exception_ptr (Exception::from_string (serialized));
            }

            static void throw_with_nested (std::string const& serialized)
            {
              std::throw_with_nested (Exception::from_string (serialized));
            }
          };

          template<typename Exception>
            struct serializer
              < Exception
              , cxx17::void_t
                  < decltype (std::declval<::boost::optional<std::string>&>() = Exception::from_exception_ptr(std::declval<std::exception_ptr>()))
                  , decltype (std::declval<std::exception_ptr&>() = Exception::to_exception_ptr (std::declval<std::string>()))
                  , decltype (Exception::throw_with_nested (std::declval<std::string>()))
                  >
              >
          {
            static ::boost::optional<std::string> from_ptr (std::exception_ptr ex)
            {
              return Exception::from_exception_ptr (ex);
            }
            static std::exception_ptr to_ptr (std::string const& serialized)
            {
              return Exception::to_exception_ptr (serialized);
            }
            static void throw_with_nested (std::string const& serialized)
            {
              return Exception::throw_with_nested (serialized);
            }
          };
        }

        template<typename Exception>
          serialization_functions::value_type make_functions()
        {
          using functor = detail::serializer<Exception>;
          return { typeid (functor).name()
                 , { &functor::from_ptr
                   , &functor::to_ptr
                   , &functor::throw_with_nested
                   }
                 };
        }
      }
    }
  }
}
