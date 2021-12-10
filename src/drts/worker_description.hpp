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

#include <boost/optional.hpp>

#include <memory>
#include <string>
#include <vector>

namespace gspc
{
  struct GSPC_DLLEXPORT worker_description
  {
    worker_description() = delete;
    worker_description (worker_description const&) = delete;
    worker_description (worker_description&&);
    worker_description& operator= (worker_description const&) = delete;
    worker_description& operator= (worker_description&&) = delete;
    ~worker_description();

    //! Create a worker description.
    //! \param capabilities are all capabilities the worker has in no
    //!        particulcar order, maximum length: 64
    //! \param num_per_node is the number of worker instances per node
    //! \param max_nodes is the maximum number of nodes where the
    //!        workers are started
    //!        0 - unlimited
    //!        1 - unique
    //! \param shm_size is the working memory per instance
    //! \param socket is the number of the NUMA socket to pin the worker
    //! \param base_port is the first port to use for the workers on a
    //!        certain node, the port is incremented for each
    //!        instance, e.g.  with "base_port = 9876" and
    //!        "num_per_node = 4" the used ports are {9876, 9877,
    //!        9878, 9879}
    worker_description ( std::vector<std::string> capabilities
                       , std::size_t num_per_node
                       , std::size_t max_nodes
                       , std::size_t shm_size
                       , ::boost::optional<std::size_t> socket
                       , ::boost::optional<unsigned short> base_port
                       );

    //! Create a worker description from a string representation:
    //!   WorkerDescription -> Capabilities Socket? Instances?
    //!   Capabilities      -> Capability | Capability '+' Capabilities
    //!   Capability        -> C-identifier
    //!   Socket            -> '#' Number
    //!   Instances         -> ':' PerNode MaxNodes? Shm? Port?
    //!   PerNode           -> Num
    //!   MaxNodes          -> 'x' Num
    //!   Shm               -> ',' Num
    //!   Port              -> '/' Num
    //!   Num               -> Digit | Digit Num
    //!   Digit             -> '0' .. '9'
    //!
    //!   with the shortcut for "optional":
    //!
    //!   T? -> '' | T
    worker_description (std::string);

    //! Create a string representation as used in the constructor.
    //! It holds: worker_description . to_string == identity
    GSPC_DLLEXPORT friend std::string to_string (worker_description const&);

    struct worker_description_implementation;
    std::unique_ptr<worker_description_implementation> _;
  };
}
