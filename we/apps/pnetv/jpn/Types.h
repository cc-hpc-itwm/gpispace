#pragma once

#include <jpn/config.h>

#include <we/type/id.hpp>

namespace jpn {

  typedef petri_net::pid_t PlaceId; ///< Place identifier.
  typedef petri_net::tid_t TransitionId; ///< Transition identifier.
  typedef int TokenCount; ///< Integer for token count.

} // namespace jpn

/* vim:set et sts=4 sw=4: */
