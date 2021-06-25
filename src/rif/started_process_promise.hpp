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

#include <gspc/detail/dllexport.hpp>

#include <exception>
#include <string>
#include <vector>

namespace fhg
{
  namespace rif
  {
    struct GSPC_DLLEXPORT started_process_promise
    {
      //! \note will remove special arguments
      started_process_promise (int& argc, char**& argv);
      ~started_process_promise();

      void set_result (std::vector<std::string> messages);
      template<typename... String>
        void set_result (String... messages);
      void set_exception (std::exception_ptr exception);

      static std::string end_sentinel_value() { return "DONE"; }

    private:
      template<typename T> void send (bool result, T const&);

      char** _original_argv;
      int& _argc;
      char**& _argv;
      std::vector<char*> _replacement_argv;
      int _startup_pipe_fd;
    };
  }
}

#include <rif/started_process_promise.ipp>
