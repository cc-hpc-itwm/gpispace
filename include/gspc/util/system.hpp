#pragma once

#include <string>


  namespace gspc::util
  {
    using Description = std::string;
    using Command = std::string;

    template<typename STRable>
      void system (STRable);

    template<>
      void system<Command> (Command);

    template<typename Exception, typename Command>
      void system (Description, Command);
  }


#include <gspc/util/system.ipp>
