#include <exception>
#include <stdexcept>


  namespace gspc::util
  {
    template<typename STRable>
      void system (STRable command)
    {
      return system (command.str());
    }

    template<typename Exception, typename Command>
      void system (Description description, Command command)
    {
      try
      {
        return system (command);
      }
      catch (...)
      {
        std::throw_with_nested (Exception {"Could not " + description});
      }
    }
  }
