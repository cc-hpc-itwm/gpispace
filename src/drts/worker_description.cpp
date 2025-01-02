// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <drts/worker_description.hpp>

#include <drts/private/worker_description_implementation.hpp>

#include <fhg/util/num.hpp>
#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/require.hpp>

#include <util-generic/join.hpp>
#include <util-generic/split.hpp>

#include <sstream>
#include <stdexcept>
#include <utility>

namespace gspc
{
  worker_description::worker_description
    ( std::vector<std::string> capabilities
    , std::size_t num_per_node
    , std::size_t max_nodes
    , std::size_t shm_size
    , ::boost::optional<std::size_t> socket
    , ::boost::optional<unsigned short> base_port
    )
      : worker_description
        { capabilities
        , num_per_node
        , max_nodes
        , shm_size
        , socket ? std::make_optional (*socket) : std::nullopt
        , base_port ? std::make_optional (*base_port) : std::nullopt
        }
  {}

  worker_description::worker_description
    ( std::vector<std::string> capabilities
    , std::size_t num_per_node
    , std::size_t max_nodes
    , std::size_t shm_size
    , std::optional<std::size_t> socket
    , std::optional<unsigned short> base_port
    )
      : _ ( std::make_unique<worker_description_implementation>
              ( std::move (capabilities)
              , std::move (num_per_node)
              , std::move (max_nodes)
              , std::move (shm_size)
              , std::move (socket)
              , std::move (base_port)
              )
          )
  {}
  worker_description::worker_description (std::string input)
    : _ ( std::make_unique<worker_description_implementation>
            (std::move (input))
        )
  {}
  worker_description::worker_description (worker_description&&) noexcept = default;
  worker_description::~worker_description() = default;

  worker_description::worker_description_implementation::worker_description_implementation
    ( std::vector<std::string> capabilities_
    , std::size_t num_per_node_
    , std::size_t max_nodes_
    , std::size_t shm_size_
    , std::optional<std::size_t> socket_
    , std::optional<unsigned short> base_port_
    )
      : capabilities (std::move (capabilities_))
      , num_per_node (std::move (num_per_node_))
      , max_nodes (std::move (max_nodes_))
      , shm_size (std::move (shm_size_))
      , socket (std::move (socket_))
      , base_port (std::move (base_port_))
  {}

  worker_description::worker_description_implementation::worker_description_implementation
    (std::string cap_spec)
      : capabilities()
      , num_per_node (1)
      , max_nodes (0)
      , shm_size (0)
      , socket()
      , base_port()
  {
    using fhg::util::parse::require::skip_spaces;
    using fhg::util::parse::require::identifier;
    using fhg::util::read_ulong;

    fhg::util::parse::position input (cap_spec);

    auto const peek
      ( [&] (char c)
        {
          if (*input == c)
          {
            ++input;

            return true;
          }

          return false;
        }
      );

    capabilities.emplace_back (identifier (input));

    while (peek ('+')) { capabilities.emplace_back (identifier (input)); }
    if (peek ('#')) { socket = read_ulong (input); }
    if (peek (':'))
    {
      num_per_node = read_ulong (input);

      if (peek ('x')) { max_nodes = read_ulong (input); }
      if (peek (',')) { shm_size = read_ulong (input); }
      if (peek ('/')) { base_port = read_ulong (input); }
    }

    if (!input.end())
    {
      throw std::invalid_argument
        (input.error_message ("Invalid capability specification"));
    }
  }

  std::string to_string (worker_description const& wd)
  {
    std::ostringstream oss;
    oss << fhg::util::join (wd._->capabilities, '+');

    if (wd._->socket)    { oss << '#' << wd._->socket.value();    }
                           oss << ':' << wd._->num_per_node;
                           oss << 'x' << wd._->max_nodes;
                           oss << ',' << wd._->shm_size;
    if (wd._->base_port) { oss << '/' << wd._->base_port.value(); }

    return oss.str();
  }
}
