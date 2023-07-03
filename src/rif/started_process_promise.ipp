// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

namespace fhg
{
  namespace rif
  {
    template<typename... String>
      void started_process_promise::set_result (String... messages)
    {
      return set_result (std::vector<std::string> {messages...});
    }
  }
}
