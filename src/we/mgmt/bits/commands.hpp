// alexander.petry@itwm.fraunhofer.de

#ifndef WE_MGMT_BITS_COMMANDS_HPP
#define WE_MGMT_BITS_COMMANDS_HPP 1

#include <we/type/id.hpp>

#include <boost/function.hpp>

namespace we
{
  namespace mgmt
  {
    namespace detail
    {
      namespace commands
      {
        struct command_t
        {
          typedef boost::function<void (command_t const &)> handler_type;

          command_t ()
          {}

          command_t (handler_type h)
            : handler (h)
          {}

          void handle()
          {
            handler (*this);
          }

        private:
          handler_type handler;
        };
      }
    }
  }
}

#endif
