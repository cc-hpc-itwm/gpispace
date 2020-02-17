#pragma once

#include <string>
#include <set>

namespace we
{
  using heureka_id_type = std::string;

  using heureka_ids_type = std::set<heureka_id_type>;

  using heureka_response_callback
    = std::function<void (heureka_ids_type const& ids)>;
}
