// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <logging/message.hpp>

namespace fhg
{
  namespace logging
  {
    template<typename Emitter>
      ostream_redirect::ostream_redirect ( std::ostream& ostream
                                         , Emitter& emitter
                                         , std::string category
                                         )
        : redirect
          ( ostream
          , [category, &emitter] (std::string const& line)
            {
              emitter.emit_message ({line, category});
            }
          )
    {}
  }
}
