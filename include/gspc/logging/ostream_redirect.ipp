// Copyright (C) 2018,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/logging/message.hpp>


  namespace gspc::logging
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
