#pragma once

#include <logging/legacy/event.hpp>

#include <string>

namespace fhg
{
  namespace logging
  {
    struct message
    {
      message (std::string content, std::string category);
      static message from_legacy (legacy::event const&);

      std::string _content;
      std::string _category;

      message() = default;
      template<typename Archive>
        void serialize (Archive& ar, unsigned int);
    };
  }
}

#include <logging/message.ipp>
