#pragma once

namespace libssh2
{
  //! \todo not thread safe
  struct context
  {
    context();
    ~context();
  };
}
