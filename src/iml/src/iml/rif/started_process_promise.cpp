// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <iml/rif/started_process_promise.hpp>

#include <util-generic/executable_path.hpp>
#include <util-generic/serialization/exception.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/format.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

namespace fhg
{
  namespace iml
  {
    namespace rif
    {
      void started_process_promise::set_result
        (std::vector<std::string> messages)
      {
        send (true, messages);
      }
      void started_process_promise::set_exception
        (std::exception_ptr exception)
      {
        send (false, fhg::util::serialization::exception::serialize (exception));
      }

      namespace
      {
        int parse_fd (int argc, char** argv)
        {
          std::string const usage
            (( ::boost::format ("Usage: %1% <pipefd> [args]...")
             % fhg::util::executable_path()
             ).str()
            );

          if (argc < 2)
          {
            throw std::invalid_argument (usage);
          }

          try
          {
            return std::stoi (argv[1]);
          }
          catch (...)
          {
            throw std::invalid_argument (usage);
          }
        }
      }

      //! \todo This is not too nice. Better hard code a specific file
      //! descriptor and ensure that one is used?
      started_process_promise::started_process_promise
          (int& argc, char**& argv)
        : _original_argv (argv)
        , _argc (argc)
        , _argv (argv)
        , _replacement_argv (argv, argv + argc)
        , _startup_pipe_fd (parse_fd (argc, argv))
      {
        _replacement_argv.erase (std::next (_replacement_argv.begin()));
        _argv = _replacement_argv.data();
        --_argc;
      }

      started_process_promise::~started_process_promise()
      {
        _argv = _original_argv;
        ++_argc;
      }

      template<typename T>
        void started_process_promise::send (bool result, T const& data)
      {
        ::boost::iostreams::stream<::boost::iostreams::file_descriptor_sink>
          stream (_startup_pipe_fd, ::boost::iostreams::close_handle);

        {
          ::boost::archive::text_oarchive archive (stream);
          archive & result;
          archive & data;
        }

        stream << end_sentinel_value();
      }
    }
  }
}
